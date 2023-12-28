/*Copyright 2008-2022 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include <bitmap3.h>
#include <lua_structure.h>
#include <structure.h>

//##################
//   Structure_OP
//##################

Structure_OP::Structure_OP(double x1_,double x2_,
                           double y1_,double y2_,
                           double z1_,double z2_,int mat_index_)
    :x1(x1_), x2(x2_),
     y1(y1_), y2(y2_),
     z1(z1_), z2(z2_),
     mat_index(mat_index_),
     stack_ID(-1),
     parent(nullptr)
{
}

Structure_OP::~Structure_OP()
{
}

int Structure_OP::index(double x,double y,double z)
{
    return -1;
}

//###############
//   Structure
//###############

Structure::Structure()
    :default_material(0),
     lx(300e-9), ly(300e-9), lz(300e-9),
     flip_x(false), flip_y(false), flip_z(false),
     periodic_x(0), periodic_y(0), periodic_z(0),
     L(nullptr), script("")
{
}

Structure::Structure(std::filesystem::path const &script_)
    :default_material(0),
     lx(300e-9), ly(300e-9), lz(300e-9),
     flip_x(false), flip_y(false), flip_z(false),
     periodic_x(0), periodic_y(0), periodic_z(0),
     L(nullptr), script(script_)
{
}

Structure::~Structure()
{
    if(L!=nullptr) lua_close(L);
}

void Structure::add_operation(Structure_OP *operation)
{
    operations.push_back(operation);
}

void Structure::discretize(Grid3<unsigned int> &matgrid,
                           int Nx,int Ny,int Nz,double Dx,double Dy,double Dz)
{
    matgrid.init(Nx,Ny,Nz,0);
    
    for(int i=0;i<Nx;i++)
    for(int j=0;j<Ny;j++)
    for(int k=0;k<Nz;k++)
    {
        double x=i*Dx;
        double y=j*Dy;
        double z=k*Dz;
        
        matgrid(i,j,k)=index(x,y,z);
    }
}

void Structure::finalize()
{
    if(L!=nullptr) lua_close(L);
    
    for(std::size_t i=0;i<operations.size();i++)
        delete operations[i];
    
    operations.clear();
    
    if(script.empty()) return;
    
    std::string full_script=script.generic_string();
    
    L=luaL_newstate();
    luaL_openlibs(L);
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(this));
    lua_setglobal(L,"lua_calling_class");
    
    std::filesystem::path caller_path=script;
    caller_path.remove_filename();
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(&caller_path));
    lua_setglobal(L,"lua_caller_path");
    
    lua_pushlightuserdata(L,reinterpret_cast<void*>(L));
    lua_setglobal(L,"lua_mother_state");
    
    lua_register(L,"add_block",LuaUI::structure_add_block);
    lua_register(L,"default_material",LuaUI::structure_default_material);
    lua_register(L,"add_coating",LuaUI::structure_add_coating);
    lua_register(L,"add_cone",LuaUI::structure_add_cone);
    lua_register(L,"add_cylinder",LuaUI::structure_add_cylinder);
    lua_register(L,"add_ellipsoid",LuaUI::structure_add_ellipsoid);
//    lua_register(L,"add_height_map",lop_add_height_map);
    lua_register(L,"add_layer",LuaUI::structure_add_layer);
    lua_register(L,"add_lua_def",LuaUI::structure_add_lua_def);
    lua_register(L,"add_mesh",LuaUI::structure_add_mesh);
    lua_register(L,"add_sin_layer",LuaUI::structure_add_sin_layer);
    lua_register(L,"add_sphere",LuaUI::structure_add_sphere);
    lua_register(L,"add_vect_block",LuaUI::structure_add_vect_block);
    lua_register(L,"add_vect_tri",LuaUI::structure_add_vect_tri);
    lua_register(L,"declare_parameter",LuaUI::structure_declare_parameter);
    lua_register(L,"flip",LuaUI::structure_set_flip);
    lua_register(L,"loop",LuaUI::structure_set_loop);
    lua_register(L,"random_packing",LuaUI::random_packing);
    
    lua_register(L,"nearest_integer",nearest_integer);
    lua_register(L,"trapz",lua_tools::lua_adaptive_trapeze_integral);
    
    int load_err = luaL_loadfile(L,full_script.c_str());
    
    if(load_err!=LUA_OK)
    {
             if(load_err==LUA_ERRFILE) std::cout<<"Lua file error with "<<full_script<<std::endl;
        else if(load_err==LUA_ERRSYNTAX) std::cout<<"Lua syntax error with "<<full_script<<std::endl;
        else std::cout<<"Lua error with "<<full_script<<std::endl;
        std::cin.get();
        return;
    }
    
    lua_pcall(L, 0, 0, 0);
    
    lua_getglobal(L,"lx");
    lx=lua_tonumber(L,-1);
    
    lua_getglobal(L,"ly");
    ly=lua_tonumber(L,-1);
    
    lua_getglobal(L,"lz");
    lz=lua_tonumber(L,-1);
    
    chk_var(lx);
    chk_var(ly);
    chk_var(lz);
    
    for(unsigned int i=0;i<operations.size();i++)
    {
        operations[i]->parent=this;
        operations[i]->stack_ID=i;
    }
}

int Structure::index(double x,double y,double z)
{
    return index(x,y,z,operations.size());
}

int Structure::index(double x,double y,double z,int restrict_level)
{
    if(flip_x) x=lx-x;
    if(flip_y) y=ly-y;
    if(flip_z) z=lz-z;
    
    if(periodic_x!=0) x=modulus(x,lx);
    if(periodic_y!=0) y=modulus(y,ly);
    if(periodic_z!=0) z=modulus(z,lz);
    
    for(int l=restrict_level-1;l>=0;l--)
    {
        int local_index=-1;
        
        for(int i=-periodic_x;i<=periodic_x;i++)
        for(int j=-periodic_y;j<=periodic_y;j++)
        for(int k=-periodic_z;k<=periodic_z;k++)
        {
            double x2=x+i*lx;
            double y2=y+j*ly;
            double z2=z+k*lz;
            
            local_index=operations[l]->index(x2,y2,z2);
            
            if(local_index!=-1)
                return local_index;
        }
    }
    
    return default_material;
}

void Structure::print(std::filesystem::path const &path,double Dx,double Dy,double Dz)
{
    finalize();
    
    std::filesystem::create_directories(path / "grid");
    
    int Nx=nearest_integer(lx/Dx);
    int Ny=nearest_integer(ly/Dy);
    int Nz=nearest_integer(lz/Dz);
    
    if(Nx==0) Nx=1;
    if(Ny==0) Ny=1;
    if(Nz==0) Nz=1;
    
    int i_min=default_material;
    int i_max=default_material;
    
    for(unsigned int i=0;i<operations.size();i++)
    {
        i_min=std::min(i_min,operations[i]->mat_index);
        i_max=std::max(i_max,operations[i]->mat_index);
    }
    
    if(i_max==i_min) i_max+=1;
    
    Bitmap tbmp_x(Ny,Nz);
    Bitmap tbmp_y(Nx,Nz);
    Bitmap tbmp_z(Nx,Ny);
    Grid2<double> stmp_x(Ny,Nz,0);
    Grid2<double> stmp_y(Nx,Nz,0);
    Grid2<double> stmp_z(Nx,Ny,0);
    
    ProgDisp dsp(Nz,"Printing structure");
    
    for(int k=0;k<Nz;k++)
    {
        Grid2<double> tmp(Nx,Ny,0);
        
        for(int i=0;i<Nx;i++){ for(int j=0;j<Ny;j++)
        {
            double val=(index(i*Dx,j*Dy,k*Dz)-i_min)/(i_max-i_min+0.0);
            tmp(i,j)=val;
            
            stmp_x(j,k)+=val;
            stmp_y(i,k)+=val;
            stmp_z(i,j)+=val;
        }}
        
        tbmp_z.G2degraM(tmp,(path / "grid/grid").generic_string(),k,".png",0,1);
        
        ++dsp;
    }
    
    tbmp_x.G2degra(stmp_x,(path / "grid/ga_x.png").generic_string());
    tbmp_y.G2degra(stmp_y,(path / "grid/ga_y.png").generic_string());
    tbmp_z.G2degra(stmp_z,(path / "grid/ga_z.png").generic_string());
    tbmp_z.G2BW(stmp_z,(path / "grid/ga_z_bw.png").generic_string());
}

void Structure::retrieve_nominal_size(double &lx_,double &ly_,double &lz_) const
{
    lx_=lx;
    ly_=ly;
    lz_=lz;
}

void Structure::voxelize(double Dx,double Dy,double Dz)
{
    finalize();
    
    int Nx=nearest_integer(lx/Dx);
    int Ny=nearest_integer(ly/Dy);
    int Nz=nearest_integer(lz/Dz);
    
    Grid3<unsigned int> matsgrid;
    discretize(matsgrid,Nx,Ny,Nz,Dx,Dy,Dz);
    
    double lx2=Nx*Dx;
    double ly2=Ny*Dy;
    double lz2=Nz*Dz;
    
    double lmin=std::min(lx2,std::min(ly2,lz2));
    
    lx2/=lmin;
    ly2/=lmin;
    lz2/=lmin;
    
//    for(int i=0;i<N_ext;i++)
//    {
//        std::vector<QFace> F_arr;
//        std::vector<Vertex> V_arr;
//        
//        voxelize_quad(V_arr,F_arr,matsgrid,ext_target[i]);
//        
//        for(unsigned int l=0;l<V_arr.size();l++)
//        {
//            V_arr[l].loc.x*=lx2;
//            V_arr[l].loc.y*=ly2;
//            V_arr[l].loc.z*=lz2;
//        }
//        
//        obj_file_save_quad(ext_fname[i],V_arr,F_arr);
//    }
}

