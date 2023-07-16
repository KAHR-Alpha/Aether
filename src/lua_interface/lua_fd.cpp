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

#include <fieldblock_holder.h>
#include <filehdl.h>
#include <lua_fd.h>
#include <lua_material.h>
#include <mesh_base.h>
#include <mesh_tools.h>
#include <string_tools.h>

extern const double Pi;

//####################
//      Sources
//####################

int create_source(lua_State *L)
{
    std::string type=lua_tostring(L,1);
    std::cout<<type<<std::endl;
    
    Source_generator **p_src=reinterpret_cast<Source_generator**>(lua_newuserdata(L,sizeof(Source_generator*)));
    *(p_src)=new Source_generator;
        
    luaL_getmetatable(L,"metatable_fdtd_source");
    lua_setmetatable(L,-2);
    
    Source_generator &src=**p_src;
    
         if(type=="afp_tfsf") src.type=Source_generator::SOURCE_GEN_AFP_TFSF;
    else if(type=="guided_planar") src.type=Source_generator::SOURCE_GEN_GUIDED_PLANAR;
    else if(type=="oscillator") src.type=Source_generator::SOURCE_GEN_OSCILLATOR;
    else
    {
        std::cout<<"Unknown sensor type: "<<type<<std::endl;
        std::cout<<"Press Enter to continue..."<<std::endl;
        std::cin.get();
    }
    return 1;
}

int source_set_location(lua_State *L)
{
    Source_generator **pp_src=reinterpret_cast<Source_generator**>(lua_touserdata(L,1));
    
    (*pp_src)->location_real=false;
    
    if(lua_gettop(L)==4)
    {
        (*pp_src)->x1=lua_tointeger(L,2);
        (*pp_src)->y1=lua_tointeger(L,3);
        (*pp_src)->z1=lua_tointeger(L,4);
        
        (*pp_src)->x2=(*pp_src)->x1+1;
        (*pp_src)->y2=(*pp_src)->y1+1;
        (*pp_src)->z2=(*pp_src)->z1+1;
    }
    else if(lua_gettop(L)==7)
    {
        (*pp_src)->x1=lua_tointeger(L,2);
        (*pp_src)->x2=lua_tointeger(L,3);
        
        (*pp_src)->y1=lua_tointeger(L,4);
        (*pp_src)->y2=lua_tointeger(L,5);
        
        (*pp_src)->z1=lua_tointeger(L,6);
        (*pp_src)->z2=lua_tointeger(L,7);
    }
    
    return 1;
}

int source_set_location_real(lua_State *L)
{
    Source_generator **pp_src=reinterpret_cast<Source_generator**>(lua_touserdata(L,1));
    
    (*pp_src)->location_real=true;
    
    if(lua_gettop(L)==4)
    {
        (*pp_src)->x1r=lua_tonumber(L,2);
        (*pp_src)->x2r=lua_tonumber(L,2);
        (*pp_src)->y1r=lua_tonumber(L,3);
        (*pp_src)->y2r=lua_tonumber(L,3);
        (*pp_src)->z1r=lua_tonumber(L,4);
        (*pp_src)->z2r=lua_tonumber(L,4);
    }
    else if(lua_gettop(L)==7)
    {
        (*pp_src)->x1r=lua_tonumber(L,2);
        (*pp_src)->x2r=lua_tonumber(L,3);
        (*pp_src)->y1r=lua_tonumber(L,4);
        (*pp_src)->y2r=lua_tonumber(L,5);
        (*pp_src)->z1r=lua_tonumber(L,6);
        (*pp_src)->z2r=lua_tonumber(L,7);
    }
    
    return 1;
}

int source_set_orientation(lua_State *L)
{
    Source_generator **pp_src=reinterpret_cast<Source_generator**>(lua_touserdata(L,1));
    
    if(lua_gettop(L)==2)
    {
        std::string orient_str=lua_tostring(L,2);
        
             if(is_x_pos(orient_str)) (*pp_src)->orientation=NORMAL_X;
        else if(is_y_pos(orient_str)) (*pp_src)->orientation=NORMAL_Y;
        else if(is_z_pos(orient_str)) (*pp_src)->orientation=NORMAL_Z;
        else if(is_x_neg(orient_str)) (*pp_src)->orientation=NORMAL_XM;
        else if(is_y_neg(orient_str)) (*pp_src)->orientation=NORMAL_YM;
        else if(is_z_neg(orient_str)) (*pp_src)->orientation=NORMAL_ZM;
    }
    else std::cout<<"Error, unrecognized orientation"<<std::endl;
    
    return 1;
}

void Source_generator_create_metatable(lua_State *L)
{
    create_obj_metatable(L,"metatable_fdtd_source");
    
    metatable_add_func(L,"location_grid",source_set_location);
    metatable_add_func(L,"location",source_set_location_real);
    metatable_add_func(L,"orientation",source_set_orientation);
    lua_wrapper<0,Source_generator,double,double,double>::bind(L,"guided_target",&Source_generator::set_guided_target);
    lua_wrapper<1,Source_generator,double,double>::bind(L,"spectrum",&Source_generator::set_spectrum);
    lua_wrapper<2,Source_generator,std::string>::bind(L,"polarization",&Source_generator::set_polarization);
}

//####################
//      Sensors
//####################

int create_sensor(lua_State *L)
{
    std::string type=lua_tostring(L,1);
    std::cout<<type<<std::endl;
        
    Sensor_generator **p_sens=reinterpret_cast<Sensor_generator**>(lua_newuserdata(L,sizeof(Sensor_generator*)));
    *(p_sens)=new Sensor_generator;
        
    luaL_getmetatable(L,"metatable_fdtd_sensor");
    lua_setmetatable(L,-2);
    
    Sensor_generator &sens=**p_sens;
    
         if(type=="box_poynting") sens.type=Sensor_generator::BOX_POYNTING;
    else if(type=="box_spectral_poynting") sens.type=Sensor_generator::BOX_SPECTRAL_POYNTING;
    else if(type=="diff_orders") sens.type=Sensor_generator::DIFF_ORDERS;
    else if(type=="farfield") sens.type=Sensor_generator::FARFIELD;
    else if(type=="fieldblock") sens.type=Sensor_generator::FIELDBLOCK;
    else if(type=="fieldmap") sens.type=Sensor_generator::FIELDMAP;
    else if(type=="fieldmap2") sens.type=Sensor_generator::FIELDMAP2;
    else if(type=="fieldpoint") sens.type=Sensor_generator::FIELDPOINT;
    else if(type=="movie") sens.type=Sensor_generator::MOVIE;
    else if(type=="planar_spectral_poynting") sens.type=Sensor_generator::PLANAR_SPECTRAL_POYNTING;
    else
    {
        std::cout<<"Unknown sensor type: "<<type<<std::endl;
        std::cout<<"Press Enter to continue..."<<std::endl;
        std::cin.get();
    }
    return 1;
}

int sensor_set_location(lua_State *L)
{
    Sensor_generator **pp_sens=reinterpret_cast<Sensor_generator**>(lua_touserdata(L,1));
    
    (*pp_sens)->location_real=false;
    
    if(lua_gettop(L)==4)
    {
        (*pp_sens)->x1=lua_tointeger(L,2);
        (*pp_sens)->y1=lua_tointeger(L,3);
        (*pp_sens)->z1=lua_tointeger(L,4);
    }
    else if(lua_gettop(L)==7)
    {
        (*pp_sens)->x1=lua_tointeger(L,2);
        (*pp_sens)->x2=lua_tointeger(L,3);
        
        (*pp_sens)->y1=lua_tointeger(L,4);
        (*pp_sens)->y2=lua_tointeger(L,5);
        
        (*pp_sens)->z1=lua_tointeger(L,6);
        (*pp_sens)->z2=lua_tointeger(L,7);
    }
    
    return 1;
}

int sensor_set_location_real(lua_State *L)
{
    Sensor_generator **pp_sens=reinterpret_cast<Sensor_generator**>(lua_touserdata(L,1));
    
    (*pp_sens)->location_real=true;
    
    if(lua_gettop(L)==4)
    {
        (*pp_sens)->x1r=lua_tonumber(L,2);
        (*pp_sens)->y1r=lua_tonumber(L,3);
        (*pp_sens)->z1r=lua_tonumber(L,4);
    }
    else if(lua_gettop(L)==7)
    {
        (*pp_sens)->x1r=lua_tonumber(L,2);
        (*pp_sens)->x2r=lua_tonumber(L,3);
        (*pp_sens)->y1r=lua_tonumber(L,4);
        (*pp_sens)->y2r=lua_tonumber(L,5);
        (*pp_sens)->z1r=lua_tonumber(L,6);
        (*pp_sens)->z2r=lua_tonumber(L,7);
    }
    
    return 1;
}

//int sensor_set_arbitrary_spectrum(lua_State *L)
//{
//    Sensor_generator *sens=lua_get_metapointer<Sensor_Generator>(L,1)
//    
//    int N=lua_gettop(L);
//        
//    std::vector<double> spectrum(N-1);
//    
//    for(int i=2;i<=N;i++)
//        spectrum[i-2]=lua_tonumber(L,i);
//    
//    sens->set_wavelength(lambda);
//    
//    return 0;
//}

void Sensor_generator_create_metatable(lua_State *L)
{
    create_obj_metatable(L,"metatable_fdtd_sensor");
    
    lua_wrapper<0,Sensor_generator,std::string>::bind(L,"disable",&Sensor_generator::disable_plane);
    lua_wrapper<1,Sensor_generator,std::string>::bind(L,"name",&Sensor_generator::set_name);
    lua_wrapper<2,Sensor_generator,std::string>::bind(L,"orientation",&Sensor_generator::set_orientation);
    lua_wrapper<3,Sensor_generator,int,int>::bind(L,"resolution",&Sensor_generator::set_resolution);
    lua_wrapper<4,Sensor_generator,int>::bind(L,"skip",&Sensor_generator::set_skip);
    lua_wrapper<5,Sensor_generator,double,double,int>::bind(L,"spectrum",&Sensor_generator::set_spectrum);
    lua_wrapper<6,Sensor_generator,double>::bind(L,"wavelength",&Sensor_generator::set_wavelength);
    
    metatable_add_func(L,"location_grid",sensor_set_location);
    metatable_add_func(L,"location",sensor_set_location_real);
}

//####################
//       FD
//####################

/*fdtd_structure_generator::fdtd_structure_generator()
    :set(false)
{
}

void fdtd_structure_generator::append(std::string cmd)
{
    append_cmd.push_back(cmd);
}

void fdtd_structure_generator::parameter(std::string cmd)
{
    parameter_cmd.push_back(cmd);
}*/


FD_Mode::FD_Mode()
    :type(MODE_NONE),
     polar_angle(0),
     polarization("TE"),
     Dx(5e-9), Dy(5e-9), Dz(5e-9),
     pad_xm(0), pad_xp(0),
     pad_ym(0), pad_yp(0),
     pad_zm(0), pad_zp(0),
     periodic_x(false),
     periodic_y(false),
     periodic_z(false),
     pml_xm(0), pml_xp(0),
     pml_ym(0), pml_yp(0),
     pml_zm(0), pml_zp(0),
     kappa_xm(25), kappa_xp(25),
     kappa_ym(25), kappa_yp(25),
     kappa_zm(25), kappa_zp(25),
     sigma_xm(1.0), sigma_xp(1.0),
     sigma_ym(1.0), sigma_yp(1.0),
     sigma_zm(1.0), sigma_zp(1.0),
     alpha_xm(0.2), alpha_xp(0.2),
     alpha_ym(0.2), alpha_yp(0.2),
     alpha_zm(0.2), alpha_zp(0.2)
{
}

void FD_Mode::compute_discretization(int &Nx,int &Ny,int &Nz,
                                     double lx,double ly,double lz) const
{
    Nx=std::max(1,nearest_integer(lx/Dx));
    Ny=std::max(1,nearest_integer(ly/Dy));
    Nz=std::max(1,nearest_integer(lz/Dz));
    
    chk_var(lx);
    chk_var(ly);
    chk_var(lz);
    chk_var(Nx);
    chk_var(Ny);
    chk_var(Nz);
}

double FD_Mode::get_Lx()
{
    int Nx,Ny,Nz;
    double lx,ly,lz;
    
    structure->retrieve_nominal_size(lx,ly,lz);
    compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    return Nx*Dx;
}

double FD_Mode::get_Ly()
{
    int Nx,Ny,Nz;
    double lx,ly,lz;
    
    structure->retrieve_nominal_size(lx,ly,lz);
    compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    return Ny*Dy;
}

double FD_Mode::get_Lz()
{
    int Nx,Ny,Nz;
    double lx,ly,lz;
    
    structure->retrieve_nominal_size(lx,ly,lz);
    compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    return Nz*Dz;
}

int FD_Mode::get_Nx()
{
    int Nx,Ny,Nz;
    double lx,ly,lz;
    
    structure->retrieve_nominal_size(lx,ly,lz);
    compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    return Nx;
}

int FD_Mode::get_Ny()
{
    int Nx,Ny,Nz;
    double lx,ly,lz;
    
    structure->retrieve_nominal_size(lx,ly,lz);
    compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    return Ny;
}

int FD_Mode::get_Nz()
{
    int Nx,Ny,Nz;
    double lx,ly,lz;
    
    structure->retrieve_nominal_size(lx,ly,lz);
    compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    return Nz;
}

void FD_Mode::set_discretization(double D_) { Dx=Dy=Dz=D_; }
void FD_Mode::set_discretization_x(double Dx_) { Dx=Dx_; }
void FD_Mode::set_discretization_y(double Dy_) { Dy=Dy_; }
void FD_Mode::set_discretization_z(double Dz_) { Dz=Dz_; }

void FD_Mode::set_material(int mat_index,std::filesystem::path const &mat_file)
{
    #ifdef OLDMAT
    for(unsigned int i=0;i<materials_index.size();i++)
    {
        if(materials_index[i]==mat_index)
        {
            materials_str[i]=mat_file;
            return;
        }
    }
    
    materials_index.push_back(mat_index);
    materials_str.push_back(mat_file);
    #endif
    
    if(mat_index<0) return;
    else if(mat_index>=static_cast<int>(materials.size()))
        materials.resize(mat_index+1);
    
    lua_material::Loader loader;
    loader.load(&materials[mat_index],mat_file);
}

void FD_Mode::set_padding(int pad_xm_,int pad_xp_,int pad_ym_,int pad_yp_,int pad_zm_,int pad_zp_)
{
    pad_xm=pad_xm_; pad_xp=pad_xp_;
    pad_ym=pad_ym_; pad_yp=pad_yp_;
    pad_zm=pad_zm_; pad_zp=pad_zp_;
    
    std::cout<<"Setting the window padding to"<<std::endl;
    std::cout<<"     X( "<<pad_xm<<" , "<<pad_xp<<" )"<<std::endl;
    std::cout<<"     Y( "<<pad_ym<<" , "<<pad_yp<<" )"<<std::endl;
    std::cout<<"     Z( "<<pad_zm<<" , "<<pad_zp<<" )"<<std::endl;
}

void FD_Mode::set_pml_xm(int N_pml,double kap,double sig,double alp)
{
    pml_xm=N_pml;
    kappa_xm=kap;
    sigma_xm=sig;
    alpha_xm=alp;
    periodic_x=false;
    pad_xm=std::max(pad_xm,5);
    
    std::cout<<"Setting the lower X PML to "<<N_pml<<" cells with"<<std::endl;
    std::cout<<"     kappa= "<<kappa_xm<<std::endl;
    std::cout<<"     sigma= "<<sigma_xm<<std::endl;
    std::cout<<"     alpha= "<<alpha_xm<<std::endl;
}

void FD_Mode::set_pml_xp(int N_pml,double kap,double sig,double alp)
{
    pml_xp=N_pml;
    kappa_xp=kap;
    sigma_xp=sig;
    alpha_xp=alp;
    periodic_x=false;
    pad_xp=std::max(pad_xp,5);
    
    std::cout<<"Setting the upper X PML to "<<N_pml<<" cells with"<<std::endl;
    std::cout<<"     kappa= "<<kappa_xp<<std::endl;
    std::cout<<"     sigma= "<<sigma_xp<<std::endl;
    std::cout<<"     alpha= "<<alpha_xp<<std::endl;
}

void FD_Mode::set_pml_ym(int N_pml,double kap,double sig,double alp)
{
    pml_ym=N_pml;
    kappa_ym=kap;
    sigma_ym=sig;
    alpha_ym=alp;
    periodic_y=false;
    pad_ym=std::max(pad_ym,5);
    
    std::cout<<"Setting the lower Y PML to "<<N_pml<<" cells with"<<std::endl;
    std::cout<<"     kappa= "<<kappa_ym<<std::endl;
    std::cout<<"     sigma= "<<sigma_ym<<std::endl;
    std::cout<<"     alpha= "<<alpha_ym<<std::endl;
}

void FD_Mode::set_pml_yp(int N_pml,double kap,double sig,double alp)
{
    pml_yp=N_pml;
    kappa_yp=kap;
    sigma_yp=sig;
    alpha_yp=alp;
    periodic_y=false;
    pad_yp=std::max(pad_yp,5);
    
    std::cout<<"Setting the upper Y PML to "<<N_pml<<" cells with"<<std::endl;
    std::cout<<"     kappa= "<<kappa_yp<<std::endl;
    std::cout<<"     sigma= "<<sigma_yp<<std::endl;
    std::cout<<"     alpha= "<<alpha_yp<<std::endl;
}

void FD_Mode::set_pml_zm(int N_pml,double kap,double sig,double alp)
{
    pml_zm=N_pml;
    kappa_zm=kap;
    sigma_zm=sig;
    alpha_zm=alp;
    periodic_z=false;
    pad_zm=std::max(pad_zm,5);
    
    std::cout<<"Setting the lower Z PML to "<<N_pml<<" cells with"<<std::endl;
    std::cout<<"     kappa= "<<kappa_zm<<std::endl;
    std::cout<<"     sigma= "<<sigma_zm<<std::endl;
    std::cout<<"     alpha= "<<alpha_zm<<std::endl;
}

void FD_Mode::set_pml_zp(int N_pml,double kap,double sig,double alp)
{
    pml_zp=N_pml;
    kappa_zp=kap;
    sigma_zp=sig;
    alpha_zp=alp;
    periodic_z=false;
    pad_zp=std::max(pad_zp,5);
    
    std::cout<<"Setting the upper Z PML to "<<N_pml<<" cells with"<<std::endl;
    std::cout<<"     kappa= "<<kappa_zp<<std::endl;
    std::cout<<"     sigma= "<<sigma_zp<<std::endl;
    std::cout<<"     alpha= "<<alpha_zp<<std::endl;
}

void FD_Mode::recompute_padding()
{
}

void FD_Mode::reset()
{
    type=MODE_NONE;
    polar_angle=0;
    polarization="TE";
    pad_xm=0; pad_xp=0;
    pad_ym=0; pad_yp=0;
    pad_zm=0; pad_zp=0;
    periodic_x=true;
    periodic_y=true;
    periodic_z=true;
    pml_xm=0; pml_xp=0;
    pml_ym=0; pml_yp=0;
    pml_zm=0; pml_zp=0;
    kappa_xm=25; kappa_xp=25;
    kappa_ym=25; kappa_yp=25;
    kappa_zm=25; kappa_zp=25;
    sigma_xm=1.0; sigma_xp=1.0;
    sigma_ym=1.0; sigma_yp=1.0;
    sigma_zm=1.0; sigma_zp=1.0;
    alpha_xm=0.2; alpha_xp=0.2;
    alpha_ym=0.2; alpha_yp=0.2;
    alpha_zm=0.2; alpha_zp=0.2;
    
    #ifdef OLDMAT
    materials_index.clear();
    materials_str.clear();
    #endif
    materials.clear();
    disable_fields.clear();
}

void FD_Mode::show() const
{
    std::cout<<"FD Mmode"<<std::endl;
    
    chk_msg_sc(type);
    chk_msg_sc(prefix);
    chk_msg_sc(directory);
    chk_msg_sc(polar_angle);
    chk_msg_sc(polarization);
    
    chk_msg_sc(pad_xm);
    chk_msg_sc(pad_xp);
    chk_msg_sc(pad_ym);
    chk_msg_sc(pad_yp);
    chk_msg_sc(pad_zm);
    chk_msg_sc(pad_zp);
    
    chk_msg_sc(pml_xm);
    chk_msg_sc(pml_xp);
    chk_msg_sc(pml_ym);
    chk_msg_sc(pml_yp);
    chk_msg_sc(pml_zm);
    chk_msg_sc(pml_zp);
    
    chk_msg_sc(kappa_xm);
    chk_msg_sc(kappa_xp);
    chk_msg_sc(kappa_ym);
    chk_msg_sc(kappa_yp);
    chk_msg_sc(kappa_zm);
    chk_msg_sc(kappa_zp);
    
    chk_msg_sc(sigma_xm);
    chk_msg_sc(sigma_xp);
    chk_msg_sc(sigma_ym);
    chk_msg_sc(sigma_yp);
    chk_msg_sc(sigma_zm);
    chk_msg_sc(sigma_zp);
    
    chk_msg_sc(alpha_xm);
    chk_msg_sc(alpha_xp);
    chk_msg_sc(alpha_ym);
    chk_msg_sc(alpha_yp);
    chk_msg_sc(alpha_zm);
    chk_msg_sc(alpha_zp);
    
    #ifdef OLDMAT
    for(unsigned int i=0;i<materials_str.size();i++)
    {
        chk_msg_sc(materials_index[i]);
        chk_msg_sc(materials_str[i]);
    }
    #endif
}

void FD_Mode::set_polarization(double polar)
{
    polar_angle=polar;
    polarization="mix";
}
void FD_Mode::set_polarization(std::string polar) { polarization=polar; }
void FD_Mode::set_prefix(std::string name) { prefix=name; }

void FD_Mode::set_output_directory(std::string dir) { directory=dir; }

void FD_Mode::set_structure(Structure *structure_)
{
    structure=structure_;
    structure->finalize();
}

void FD_Mode::process()
{
}

int FD_mode_disable_fields(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    int N=lua_gettop(L);
    
    for(int i=2;i<=N;i++)
    {
        std::string str=lua_tostring(L,i);
        
             if(str=="ex" || str=="Ex" || str=="EX" || str=="eX")
        {
            (*pp_fd)->disable_fields.push_back(EX_FIELD);
            std::cout<<"Disabling the Ex field"<<std::endl;
        }
        else if(str=="ey" || str=="Ey" || str=="EY" || str=="eY")
        {
            (*pp_fd)->disable_fields.push_back(EY_FIELD);
            std::cout<<"Disabling the Ey field"<<std::endl;
        }
        else if(str=="ez" || str=="Ez" || str=="EZ" || str=="eZ")
        {
            (*pp_fd)->disable_fields.push_back(EZ_FIELD);
            std::cout<<"Disabling the Ez field"<<std::endl;
        }
        else if(str=="hx" || str=="Hx" || str=="HX" || str=="hX")
        {
            (*pp_fd)->disable_fields.push_back(HX_FIELD);
            std::cout<<"Disabling the Hx field"<<std::endl;
        }
        else if(str=="hy" || str=="Hy" || str=="HY" || str=="hY")
        {
            (*pp_fd)->disable_fields.push_back(HY_FIELD);
            std::cout<<"Disabling the Hy field"<<std::endl;
        }
        else if(str=="hz" || str=="Hz" || str=="HZ" || str=="hZ")
        {
            (*pp_fd)->disable_fields.push_back(HZ_FIELD);
            std::cout<<"Disabling the Hz field"<<std::endl;
        }
    }
    
    return 1;
}

int FD_mode_get_lx(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    lua_pushnumber(L,(*pp_fd)->get_Lx());
    
    return 1;
}

int FD_mode_get_ly(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    lua_pushnumber(L,(*pp_fd)->get_Ly());
    
    return 1;
}

int FD_mode_get_lz(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    lua_pushnumber(L,(*pp_fd)->get_Lz());
    
    return 1;
}

int FD_mode_get_nx(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    lua_pushinteger(L,(*pp_fd)->get_Nx());
    
    return 1;
}

int FD_mode_get_ny(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    lua_pushinteger(L,(*pp_fd)->get_Ny());
    
    return 1;
}

int FD_mode_get_nz(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    lua_pushinteger(L,(*pp_fd)->get_Nz());
    
    return 1;
}

int FD_mode_set_material(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    int mat_index=lua_tointeger(L,2);
    std::string mat_file=lua_tostring(L,3);
    
    lua_getglobal(L,"lua_caller_path");
    std::filesystem::path *caller_path=reinterpret_cast<std::filesystem::path*>(lua_touserdata(L,-1));
    std::filesystem::path mat_path=PathManager::locate_file(mat_file,*caller_path);
    
    std::cout<<"Setting the material "<<mat_index<<" to "<<mat_path<<std::endl;
    
    (*pp_fd)->set_material(mat_index,mat_path);
    
    return 1;
}

int FD_mode_set_output_directory(lua_State *L)
{
//    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    FD_Mode *p_fd=lua_get_metapointer<FD_Mode>(L,1);
    
    std::string directory=lua_tostring(L,2);
    
    proper_dirname_convertion(directory);
    
    std::cout<<"Setting the results output directory to "<<directory<<std::endl;
    
    p_fd->directory=directory;
    
    return 1;
}

int FD_mode_set_polarization(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    if(lua_isnumber(L,2))
    {
        double polar=lua_tonumber(L,2);
        
        std::cout<<"Setting the polarization to "<<polar<<" degrees with respect to TE"<<std::endl;
        (*pp_fd)->set_polarization(polar);
    }
    else
    {
        std::string polarization="TE";
        std::string tmp=lua_tostring(L,2);
        
        if(tmp=="te" || tmp=="TE" || tmp=="Te" || tmp=="tE") polarization="TE";
        if(tmp=="tm" || tmp=="TM" || tmp=="Tm" || tmp=="tM") polarization="TM";
        
        std::cout<<"Setting the polarization to "<<polarization<<std::endl;
        
        (*pp_fd)->set_polarization(polarization);
    }
    
    return 1;
}

int FD_mode_set_prefix(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    
    std::string prefix=lua_tostring(L,2);
    
    int N=prefix.size();
    if(prefix[N-1]!='_') prefix.append("_");
    
    std::cout<<"Setting the simulation prefix to "<<prefix<<std::endl;
    
    (*pp_fd)->set_prefix(prefix);
    
    return 1;
}

int FD_mode_set_structure(lua_State *L)
{
    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
    Structure *p_structure=lua_get_metapointer<Structure>(L,2);
    
    (*pp_fd)->set_structure(p_structure);
    
    std::cout<<"Setting the structure to "<<p_structure->script<<std::endl;
    
    return 1;
}

//int FD_mode_set_threads(lua_State *L)
//{
//    FD_Mode **pp_fd=reinterpret_cast<FD_Mode**>(lua_touserdata(L,1));
//    
//    (*pp_fd)->Nthreads=lua_tointeger(L,2);
//    
//    std::cout<<"Setting the maximum number of threads to "<<(*pp_fd)->Nthreads<<std::endl;
//    
//    return 1;
//}

//####################
//     FDFD Mode
//####################

FDFD_Mode::FDFD_Mode()
    :FD_Mode(),
     N_theta(1), theta_min(0), theta_max(0),
     N_phi(1), phi_min(0), phi_max(0),
     Nl(49), lambda_min(370e-9), lambda_max(850e-9),
     solver("LU"),
     output_diffraction(false),
     output_map(false)
{
}

void FDFD_Mode::process()
{
    if(type==FDFD)
    {
        mode_fdfd(*this);
    }
}

void FDFD_Mode::set_azimuth(Angle phi_min_,Angle phi_max_,int N_phi_)
{
    phi_min=phi_min_;
    phi_max=phi_max_;
    N_phi=N_phi_;
}

void FDFD_Mode::set_incidence(Angle theta_min_,Angle theta_max_,int N_theta_)
{
    theta_min=theta_min_;
    theta_max=theta_max_;
    N_theta=N_theta_;
}

void FDFD_Mode::set_spectrum(double lambda_min_,double lambda_max_,int Nl_)
{
    lambda_min=lambda_min_;
    lambda_max=lambda_max_;
    Nl=Nl_;
}

int FDFD_mode_set_azimuth(lua_State *L)
{
    FDFD_Mode **pp_fdfd=reinterpret_cast<FDFD_Mode**>(lua_touserdata(L,1));
    
    Angle phi_min=Degree(lua_tonumber(L,2));
    Angle phi_max=Degree(lua_tonumber(L,3));
    int N_phi=lua_tointeger(L,4);
    
    (*pp_fdfd)->set_azimuth(phi_min,phi_max,N_phi);
    
    return 1;
}

int FDFD_mode_set_incidence(lua_State *L)
{
    FDFD_Mode **pp_fdfd=reinterpret_cast<FDFD_Mode**>(lua_touserdata(L,1));
    
    Angle theta_min=Degree(lua_tonumber(L,2));
    Angle theta_max=Degree(lua_tonumber(L,3));
    int N_theta=lua_tointeger(L,4);
    
    (*pp_fdfd)->set_incidence(theta_min,theta_max,N_theta);
    
    return 1;
}

int FDFD_mode_output_diffraction(lua_State *L)
{
    FDFD_Mode **pp_fdfd=reinterpret_cast<FDFD_Mode**>(lua_touserdata(L,1));
    
    std::cout<<"Enabling diffracted orders computation"<<std::endl;
    
    (*pp_fdfd)->output_diffraction=true;
    
    return 1;
}

int FDFD_mode_output_map(lua_State *L)
{
    FDFD_Mode **pp_fdfd=reinterpret_cast<FDFD_Mode**>(lua_touserdata(L,1));
    
    std::cout<<"Enabling fieldmap"<<std::endl;
    
    (*pp_fdfd)->output_map=true;
    
    return 1;
}

int FDFD_mode_set_solver(lua_State *L)
{
    FDFD_Mode **pp_fdfd=reinterpret_cast<FDFD_Mode**>(lua_touserdata(L,1));
    
    std::string solver=lua_tostring(L,2);
    
    (*pp_fdfd)->solver=solver;
    
    return 1;
}

int FDFD_mode_set_spectrum(lua_State *L)
{
    FDFD_Mode **pp_fdfd=reinterpret_cast<FDFD_Mode**>(lua_touserdata(L,1));
    
    double lambda_min=lua_tonumber(L,2);
    double lambda_max=lua_tonumber(L,3);
    int Nl=lua_tointeger(L,4);
    
    (*pp_fdfd)->set_spectrum(lambda_min,lambda_max,Nl);
    
    return 1;
}

//###################
//     FDMS_Mode
//###################

FDMS_Mode::FDMS_Mode()
    :FD_Mode(),
     Nl(481),
     lambda_min(370e-9), lambda_max(850e-9),
     lambda_target(500e-9), nr_target(1.0), ni_target(0.0),
     solver("default")
{
}

void FDMS_Mode::set_solver(std::string solver_)
{
    if(solver_=="bicgstab") solver="bicgstab";
}

void FDMS_Mode::set_spectrum(double lambda_min_,double lambda_max_,double Nl_)
{
    lambda_min=lambda_min_;
    lambda_max=lambda_max_;
    Nl=Nl_;
}

void FDMS_Mode::set_target(double lambda_target_,double nr_target_,double ni_target_)
{
    lambda_target=lambda_target_;
    nr_target=nr_target_;
    ni_target=ni_target_;
}

//####################
//     FDTD Mode
//####################

FDTD_Mode::FDTD_Mode()
    :FD_Mode(),
     Nt(5000), tapering(0),
     display_step(-1),
     time_type(TIME_FIXED), 
     time_mod(1.0),
     cc_step(500),
     cc_lmin(370e-9), cc_lmax(850e-9),
     cc_coeff(1e-3), cc_quant(500),
     cc_layout("nnb"),
     Nl(481), lambda_min(370e-9), lambda_max(850e-9),
     obl_phase_type(0), obl_phase_Nkp(1), obl_phase_skip(0),
     obl_phase_kp_ic(0), obl_phase_kp_fc(1.0),
     obl_phase_phi(0),
     obl_phase_amin(0), obl_phase_amax(0),
     obl_phase_lmin(500e-9), obl_phase_lmax(500e-9),
     obl_phase_cut_angle(Degree(75)),
     obl_phase_safe_angle(Degree(65))
{
    type=FDTD_CUSTOM;
}

void FDTD_Mode::add_sensor(Sensor_generator const &sens)
{
    sensors.push_back(sens);
}

void FDTD_Mode::add_source(Source_generator const &src)
{
    sources.push_back(src);
}

void FDTD_Mode::delete_sensor(unsigned int ID)
{
    if(ID>=sensors.size()) return;
    
    std::vector<Sensor_generator>::const_iterator it=sensors.begin()+ID;
    
    sensors.erase(it);
}

void FDTD_Mode::delete_source(unsigned int ID)
{
    if(ID>=sources.size()) return;
    
    std::vector<Source_generator>::const_iterator it=sources.begin()+ID;
    
    sources.erase(it);
}

void FDTD_Mode::finalize()
{
    unsigned int i;
    
    int Nx,Ny,Nz;
    double lx,ly,lz;
    
    structure->retrieve_nominal_size(lx,ly,lz);
    compute_discretization(Nx,Ny,Nz,lx,ly,lz);
    
    for(i=0;i<sensors.size();i++)
        sensors[i].to_discrete(Dx,Dy,Dz);
    
    for(i=0;i<sources.size();i++)
        sources[i].to_discrete(Dx,Dy,Dz);
    
    if(type==FDTD_NORMAL || type==FDTD_OBLIQUE_ARS)
    {
        pml_xm=pml_xp=0;
        pml_ym=pml_yp=0;
        
        if(pml_zm<=0) pml_zm=25;
        if(pml_zp<=0) pml_zp=25;
    }
    
    if(Nx>1)
    {
        if(pml_xm)
        {
            pad_xm=std::max(pad_xm,5);
            
            for(i=0;i<sensors.size();i++)
                pad_xm=std::max(pad_xm,5-sensors[i].x1);
            
            for(i=0;i<sources.size();i++)
            {
                if(sources[i].type==Source_generator::SOURCE_GEN_OSCILLATOR)
                    pad_xm=std::max(pad_xm,50-sources[i].x1);
                else pad_xm=std::max(pad_xm,5-sources[i].x1);
            }
        }
        else pad_xm=0;
        
        if(pml_xp)
        {
            pad_xp=std::max(pad_xp,5);
            
            for(i=0;i<sensors.size();i++)
                pad_xp=std::max(pad_xp,5+sensors[i].x2-Nx);
            
            for(i=0;i<sources.size();i++)
            {
                if(sources[i].type==Source_generator::SOURCE_GEN_OSCILLATOR)
                    pad_xp=std::max(pad_xp,50+sources[i].x2-Nx);
                else pad_xp=std::max(pad_xp,5+sources[i].x2-Nx);
            }
        }
        else pad_xp=0;
    }
    
    if(Ny>1)
    {
        if(pml_ym)
        {
            pad_ym=std::max(pad_ym,5);
            
            for(i=0;i<sensors.size();i++)
                pad_ym=std::max(pad_ym,5-sensors[i].y1);
            
            for(i=0;i<sources.size();i++)
            {
                if(sources[i].type==Source_generator::SOURCE_GEN_OSCILLATOR)
                    pad_ym=std::max(pad_ym,50-sources[i].y1);
                else pad_ym=std::max(pad_ym,5-sources[i].y1);
            }
        }
        else pad_ym=0;
        
        if(pml_yp)
        {
            pad_yp=std::max(pad_yp,5);
            
            for(i=0;i<sensors.size();i++)
                pad_yp=std::max(pad_yp,5+sensors[i].y2-Ny);
            
            for(i=0;i<sources.size();i++)
            {
                if(sources[i].type==Source_generator::SOURCE_GEN_OSCILLATOR)
                    pad_yp=std::max(pad_yp,50+sources[i].y2-Ny);
                else pad_yp=std::max(pad_yp,5+sources[i].y2-Ny);
            }
        }
        else pad_yp=0;
    }
    
    if(Nz>1)
    {
        if(pml_zm)
        {
            pad_zm=std::max(pad_zm,5);
            
            for(i=0;i<sensors.size();i++)
                pad_zm=std::max(pad_zm,5-sensors[i].z1);
            
            for(i=0;i<sources.size();i++)
            {
                if(sources[i].type==Source_generator::SOURCE_GEN_OSCILLATOR)
                    pad_zm=std::max(pad_zm,50-sources[i].z1);
                else pad_zm=std::max(pad_zm,5-sources[i].z1);
            }
        }
        else pad_zm=0;
        
        if(pml_zp)
        {
            pad_zp=std::max(pad_zp,5);
            
            for(i=0;i<sensors.size();i++)
                pad_zp=std::max(pad_zp,5+sensors[i].z2-Nz);
            
            for(i=0;i<sources.size();i++)
            {
                if(sources[i].type==Source_generator::SOURCE_GEN_OSCILLATOR)
                    pad_zp=std::max(pad_zp,50+sources[i].z2-Nz);
                else pad_zp=std::max(pad_zp,5+sources[i].z2-Nz);
            }
        }
        else pad_zp=0;
    }
}

void FDTD_Mode::finalize_thight()
{
    if(type==FDTD_NORMAL || type==FDTD_OBLIQUE_ARS)
    {
        pml_xm=pml_xp=0;
        pml_ym=pml_yp=0;
        
        if(pml_zm<=0) pml_zm=25;
        if(pml_zp<=0) pml_zp=25;
    }
    
//    if(Nx>1)
    {
        if(pml_xm) pad_xm=5;
        else pad_xm=0;
        
        if(pml_xp) pad_xp=5;
        else pad_xp=0;
    }
    
//    if(Ny>1)
    {
        if(pml_ym) pad_ym=5;
        else pad_ym=0;
        
        if(pml_yp)pad_yp=5;
        else pad_yp=0;
    }
    
//    if(Nz>1)
    {
        if(pml_zm) pad_zm=5;
        else pad_zm=0;
        
        if(pml_zp) pad_zp=5;
        else pad_zp=0;
    }
    
    finalize();
}

void FDTD_Mode::reset()
{
    FD_Mode::reset();
    
    Nt=5000; tapering=0;
    display_step=-1;
    time_type=TIME_FIXED; 
    time_mod=1.0;
    cc_step=500;
    cc_lmin=370e-9; cc_lmax=850e-9;
    cc_coeff=1e-3; cc_quant=500;
    cc_layout="nnb";
    Nl=481; lambda_min=370e-9; lambda_max=850e-9;
    obl_phase_type=0; obl_phase_Nkp=1; obl_phase_skip=0;
    obl_phase_kp_ic=0; obl_phase_kp_fc=1.0;
    obl_phase_phi=0;
    obl_phase_amin=0; obl_phase_amax=0;
    obl_phase_lmin=500e-9; obl_phase_lmax=500e-9;
    obl_phase_cut_angle=Degree(75);
    obl_phase_safe_angle=Degree(65);

    sensors.clear();
    sources.clear();
}

void FDTD_Mode::set_auto_tsteps(int Nt_,int cc_step_,double cc_coeff_)
{
    Nt=Nt_;
    cc_step=cc_step_;
    cc_coeff=cc_coeff_;
    
    time_type=TIME_ENERGY;
}

void FDTD_Mode::set_auto_tsteps(int Nt_,int cc_step_,
                                double cc_lmin_,double cc_lmax_,
                                double cc_coeff_,int cc_quant_,
                                std::string const &cc_layout_)
{
    Nt=Nt_;
    cc_step=cc_step_;
    cc_lmin=cc_lmin_;
    cc_lmax=cc_lmax_;
    cc_coeff=cc_coeff_;
    cc_quant=cc_quant_;
    cc_layout=cc_layout_;
    
    time_type=TIME_FT;
}

void FDTD_Mode::process()
{
    finalize();
    
    if(type==FDTD_NORMAL)
    {
        FDTD_normal_incidence(*this);
    }
    else if(type==FDTD_OBLIQUE_ARS)
    {
        FDTD_oblique_biphase(*this);
    }
    else if(type==FDTD_SINGLE_PARTICLE)
    {
        FDTD_single_particle(*this);
    }
    else if(type==FDTD_CUSTOM)
    {
        mode_default_fdtd(*this);
    }
    else if(type==FDTD_LAB)
    {
        mode_fdtd_lab(*this);
    }
}

void FDTD_Mode::set_N_tsteps(int Nt_)
{
    Nt=Nt_;
    time_type=TIME_FIXED;
    
    std::cout<<"Setting the number of time steps to "<<Nt<<std::endl;
}

void FDTD_Mode::set_display_step(int N)
{
    display_step=N;
}

void FDTD_Mode::set_spectrum(double lambda_min_,double lambda_max_,int Nl_)
{
    lambda_min=lambda_min_;
    lambda_max=lambda_max_;
    Nl=Nl_;
}

void FDTD_Mode::set_time_mod(double md) { time_mod=md; }

void FDTD_Mode::show() const
{
    FD_Mode::show();
    
    std::cout<<"FDTD Mode"<<std::endl;
    chk_msg_sc(Nt);
    chk_msg_sc(display_step);
    chk_msg_sc(time_type);
    chk_msg_sc(time_mod);
    chk_msg_sc(cc_step);
    chk_msg_sc(cc_lmin);
    chk_msg_sc(cc_lmax);
    chk_msg_sc(cc_coeff);
    chk_msg_sc(cc_quant);
    chk_msg_sc(Nl);
    chk_msg_sc(lambda_min);
    chk_msg_sc(lambda_max);
}

void FDTD_Mode_create_metatable(lua_State *L)
{
    create_obj_metatable(L,"metatable_fdtd");
    
    metatable_add_func(L,"auto_tsteps",FDTD_mode_set_auto_tsteps);
    metatable_add_func(L,"display_step",FDTD_mode_set_display_step);
    lua_wrapper<1,FDTD_Mode,double>::bind(L,"Dx",&FDTD_Mode::set_discretization_x);
    lua_wrapper<2,FDTD_Mode,double>::bind(L,"Dxyz",&FDTD_Mode::set_discretization);
    lua_wrapper<3,FDTD_Mode,double>::bind(L,"Dy",&FDTD_Mode::set_discretization_y);
    lua_wrapper<4,FDTD_Mode,double>::bind(L,"Dz",&FDTD_Mode::set_discretization_z);
    metatable_add_func(L,"Lx",FD_mode_get_lx);
    metatable_add_func(L,"Ly",FD_mode_get_ly);
    metatable_add_func(L,"Lz",FD_mode_get_lz);
    metatable_add_func(L,"material",FD_mode_set_material);
    lua_wrapper<5,FDTD_Mode,int>::bind(L,"N_tsteps",&FDTD_Mode::set_N_tsteps);
    metatable_add_func(L,"Nx",FD_mode_get_nx);
    metatable_add_func(L,"Ny",FD_mode_get_ny);
    metatable_add_func(L,"Nz",FD_mode_get_nz);
    metatable_add_func(L,"output_directory",FD_mode_set_output_directory);
    lua_wrapper<6,FDTD_Mode,int,int,int,int,int,int>::bind(L,"padding",&FDTD_Mode::set_padding);
    lua_wrapper<7,FDTD_Mode,int,double,double,double>::bind(L,"pml_xm",&FDTD_Mode::set_pml_xm);
    lua_wrapper<8,FDTD_Mode,int,double,double,double>::bind(L,"pml_xp",&FDTD_Mode::set_pml_xp);
    lua_wrapper<9,FDTD_Mode,int,double,double,double>::bind(L,"pml_ym",&FDTD_Mode::set_pml_ym);
    lua_wrapper<10,FDTD_Mode,int,double,double,double>::bind(L,"pml_yp",&FDTD_Mode::set_pml_yp);
    lua_wrapper<11,FDTD_Mode,int,double,double,double>::bind(L,"pml_zm",&FDTD_Mode::set_pml_zm);
    lua_wrapper<12,FDTD_Mode,int,double,double,double>::bind(L,"pml_zp",&FDTD_Mode::set_pml_zp);
    metatable_add_func(L,"polarization",FD_mode_set_polarization);
    metatable_add_func(L,"prefix",FD_mode_set_prefix);
    metatable_add_func(L,"structure",FD_mode_set_structure);
    metatable_add_func(L,"tapering",FDTD_mode_set_tapering);
    metatable_add_func(L,"time_mod",FDTD_mode_set_time_mod);
    
    metatable_add_func(L,"cut_angle",FDTD_mode_obph_set_cut_angle);
    metatable_add_func(L,"kp_auto",FDTD_mode_obph_set_kp_auto);
    metatable_add_func(L,"kp_fixed_angle",FDTD_mode_obph_set_kp_fixed_angle);
    metatable_add_func(L,"kp_fixed_lambda",FDTD_mode_obph_set_kp_fixed_lambda);
    metatable_add_func(L,"kp_full",FDTD_mode_obph_set_kp_full);
    metatable_add_func(L,"kp_skip",FDTD_mode_obph_skip);
    metatable_add_func(L,"kp_target",FDTD_mode_obph_set_kp_target);
    metatable_add_func(L,"incident_plane",FDTD_mode_obph_set_phi);
    
    // Non-trivial functions
    
    metatable_add_func(L,"disable_fields",FD_mode_disable_fields);
    metatable_add_func(L,"register_sensor",FDTD_mode_register_sensor);
    metatable_add_func(L,"register_source",FDTD_mode_register_source);
    metatable_add_func(L,"spectrum",FDTD_mode_set_spectrum);
}

int FDTD_mode_register_sensor(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    Sensor_generator **pp_sens=reinterpret_cast<Sensor_generator**>(lua_touserdata(L,2));
    
    (*pp_fdtd)->add_sensor(**pp_sens);
    
    return 1;
}

int FDTD_mode_register_source(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    Source_generator **pp_src=reinterpret_cast<Source_generator**>(lua_touserdata(L,2));
    
    (*pp_fdtd)->add_source(**pp_src);
    
    return 1;
}

int FDTD_mode_set_auto_tsteps(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    if(lua_gettop(L)==4)
    {
        int Nt=lua_tointeger(L,2);
        int step=lua_tointeger(L,3);
        double coeff=lua_tonumber(L,4);
        
        (*pp_fdtd)->set_auto_tsteps(Nt,step,coeff);
    }
    else
    {
        int Nt=lua_tointeger(L,2);
        int step=lua_tointeger(L,3);
        double lmin=lua_tonumber(L,4);
        double lmax=lua_tonumber(L,5);
        double coeff=lua_tonumber(L,6);
        int quant=lua_tointeger(L,7);
        std::cout<<"Setting the number of time steps to automatic mode with coefficient "<<coeff<<std::endl;
        
        std::string cc_layout="nnb";
        
        if(lua_gettop(L)==8) cc_layout=lua_tostring(L,8);
        
        (*pp_fdtd)->set_auto_tsteps(Nt,step,lmin,lmax,coeff,quant,cc_layout);
    }
    
    return 1;
}

int FDTD_mode_set_display_step(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    int display_step=lua_tointeger(L,2);
    std::cout<<"Setting the display step to "<<display_step<<std::endl;
    
    (*pp_fdtd)->set_display_step(display_step);
    
    return 1;
}

int FDTD_mode_set_spectrum(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    double lambda_min=lua_tonumber(L,2);
    double lambda_max=lua_tonumber(L,3);
    
    if(lua_gettop(L)==3)
    {
        (*pp_fdtd)->set_spectrum(lambda_min,lambda_max);
    
        std::cout<<"Setting the spectrum between " <<add_unit_u(lambda_min)<<" and "<<add_unit_u(lambda_max)<<std::endl;
    }
    else if(lua_gettop(L)==4)
    {
        int Nl=lua_tointeger(L,4);
        (*pp_fdtd)->set_spectrum(lambda_min,lambda_max,Nl);
    
        std::cout<<"Setting the analysis to "<<Nl<<" points between "
                 <<add_unit_u(lambda_min)<<" and "<<add_unit_u(lambda_max)<<std::endl;
    }
    
    return 1;
}

int FDTD_mode_set_tapering(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    double Ntap=lua_tointeger(L,2);
    
    std::cout<<"Setting the tapering to "<<Ntap<<" time steps"<<std::endl;
    
    (*pp_fdtd)->tapering=Ntap;
    
    return 1;
}

int FDTD_mode_set_time_mod(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    double time_mod=lua_tonumber(L,2);
    
    std::cout<<"Setting the time modifier to "<<time_mod<<std::endl;
    
    (*pp_fdtd)->set_time_mod(time_mod);
    
    return 1;
}

//####################
//     FDTD Mode
//   Oblique Phase
//####################

int FDTD_mode_obph_set_cut_angle(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    FDTD_Mode &fm=**pp_fdtd;
    
    fm.obl_phase_cut_angle=Degree(lua_tonumber(L,2));
    fm.obl_phase_safe_angle=Degree(lua_tonumber(L,3));
    
    return 1;
}

int FDTD_mode_obph_set_kp_auto(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    FDTD_Mode &fm=**pp_fdtd;
    
    fm.obl_phase_type=OBL_PHASE_KP_AUTO;
    fm.obl_phase_amin=lua_tonumber(L,2)*Pi/180.0;
    fm.obl_phase_amax=lua_tonumber(L,3)*Pi/180.0;
    
    return 1;
}

int FDTD_mode_obph_set_kp_fixed_angle(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    FDTD_Mode &fm=**pp_fdtd;
    
    fm.obl_phase_type=OBL_PHASE_KP_FIXED_A;
    fm.obl_phase_Nkp=lua_tointeger(L,2);
    fm.obl_phase_lmin=lua_tonumber(L,3);
    fm.obl_phase_lmax=lua_tonumber(L,4);
    fm.obl_phase_amin=lua_tonumber(L,5)*Pi/180.0;
    fm.obl_phase_amax=lua_tonumber(L,5)*Pi/180.0;
    
    return 1;
}

int FDTD_mode_obph_set_kp_fixed_lambda(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    FDTD_Mode &fm=**pp_fdtd;
    
    fm.obl_phase_type=OBL_PHASE_KP_FIXED_L;
    fm.obl_phase_Nkp=lua_tointeger(L,2);
    fm.obl_phase_lmin=fm.obl_phase_lmax=lua_tonumber(L,3);
    fm.obl_phase_amin=lua_tonumber(L,4)*Pi/180.0;
    fm.obl_phase_amax=lua_tonumber(L,5)*Pi/180.0;
        
    return 1;
}

int FDTD_mode_obph_set_kp_full(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    FDTD_Mode &fm=**pp_fdtd;
    
    fm.obl_phase_type=OBL_PHASE_KP_FULL;
    
    fm.obl_phase_Nkp=lua_tointeger(L,2);
    fm.obl_phase_kp_ic=lua_tonumber(L,3);
    fm.obl_phase_kp_fc=lua_tonumber(L,4);
    
    return 1;
}

int FDTD_mode_obph_set_kp_target(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    FDTD_Mode &fm=**pp_fdtd;
    
    fm.obl_phase_type=OBL_PHASE_KP_TARGET;
    fm.obl_phase_Nkp=1;
    fm.obl_phase_lmin=fm.obl_phase_lmax=lua_tonumber(L,2);
    fm.obl_phase_amin=lua_tonumber(L,3)*Pi/180.0;
    fm.obl_phase_amax=lua_tonumber(L,3)*Pi/180.0;
    
    return 1;
}

int FDTD_mode_obph_set_phi(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    FDTD_Mode &fm=**pp_fdtd;
    
    fm.obl_phase_phi=Degree(lua_tonumber(L,2));
    
    return 1;
}

int FDTD_mode_obph_skip(lua_State *L)
{
    FDTD_Mode **pp_fdtd=reinterpret_cast<FDTD_Mode**>(lua_touserdata(L,1));
    
    FDTD_Mode &fm=**pp_fdtd;
    
    fm.obl_phase_skip=lua_tointeger(L,2);
    
    return 1;
}

//##############################
//       Fieldblock Treat
//##############################

Fblock_treat_mode::Fblock_treat_mode()
    :baseline(1.0), fname(""),
     blender_output(false),
     surface_poynting_compute(false),
     apply_stencil(false)
{
}

int fblock_treat_mode_apply_stencil(lua_State *L)
{
    Fblock_treat_mode **pp_fb=reinterpret_cast<Fblock_treat_mode**>(lua_touserdata(L,1));
    Fblock_treat_mode &fb=*(*pp_fb);
    
    fb.apply_stencil=true;
    fb.stencil.script=lua_tostring(L,2);
    fb.stencil_index=lua_tointeger(L,3);
    fb.stencil_output=lua_tostring(L,4);
    
    return 0;
}

int fblock_treat_mode_integrate_field(lua_State *L)
{
    return 0;
}

int fblock_treat_mode_set_baseline(lua_State *L)
{
    Fblock_treat_mode **pp_fb=reinterpret_cast<Fblock_treat_mode**>(lua_touserdata(L,1));
    
    (*pp_fb)->baseline=lua_tonumber(L,2);
    
    return 0;
}

int fblock_treat_mode_set_file(lua_State *L)
{
    Fblock_treat_mode **pp_fb=reinterpret_cast<Fblock_treat_mode**>(lua_touserdata(L,1));
    
    std::string fname=lua_tostring(L,2);
    
    (*pp_fb)->fname=fname;
    
    return 0;
}

int substract_fieldblocks(lua_State *L)
{
    std::string fname_out=lua_tostring(L,1);
    std::string fname_1=lua_tostring(L,2);
    std::string fname_2=lua_tostring(L,3);
    
    FieldBlockHolder fholder_1,fholder_2;
    
    fholder_1.load(fname_1);
    fholder_2.load(fname_2);
    
    FieldBlockHolder fholder_out(fholder_1);
    
    int i,j,k;
    
    for(i=0;i<fholder_1.Nx;i++) for(j=0;j<fholder_1.Ny;j++) for(k=0;k<fholder_1.Nz;k++)
    {
        fholder_out.Ex(i,j,k)=fholder_1.Ex(i,j,k)-fholder_2.Ex(i,j,k);
        fholder_out.Ey(i,j,k)=fholder_1.Ey(i,j,k)-fholder_2.Ey(i,j,k);
        fholder_out.Ez(i,j,k)=fholder_1.Ez(i,j,k)-fholder_2.Ez(i,j,k);
        
        fholder_out.Hx(i,j,k)=fholder_1.Hx(i,j,k)-fholder_2.Hx(i,j,k);
        fholder_out.Hy(i,j,k)=fholder_1.Hy(i,j,k)-fholder_2.Hy(i,j,k);
        fholder_out.Hz(i,j,k)=fholder_1.Hz(i,j,k)-fholder_2.Hz(i,j,k);
    }
    
    fholder_out.save(fname_out);
    
    return 1;
}

//##############################
//       Fieldmap Treat
//##############################

Fmap_treat_mode::Fmap_treat_mode()
    :baseline(1.0), fname(""),
     scale_x(1), scale_y(1), scale_z(1)
{
}

int fmap_treat_mode_set_baseline(lua_State *L)
{
    Fmap_treat_mode **pp_fm=reinterpret_cast<Fmap_treat_mode**>(lua_touserdata(L,1));
    
    (*pp_fm)->baseline=lua_tonumber(L,2);
    
    return 0;
}

int fmap_treat_mode_set_file(lua_State *L)
{
    Fmap_treat_mode **pp_fm=reinterpret_cast<Fmap_treat_mode**>(lua_touserdata(L,1));
    
    std::string fname=lua_tostring(L,2);
    
    (*pp_fm)->fname=fname;
    
    return 0;
}

int fmap_treat_mode_set_scale(lua_State *L)
{
    Fmap_treat_mode **pp_fm=reinterpret_cast<Fmap_treat_mode**>(lua_touserdata(L,1));
    
         if(lua_gettop(L)==2)
    {
        double tmp=lua_tointeger(L,2);
        
        (*pp_fm)->scale_x=tmp;
        (*pp_fm)->scale_y=tmp;
        (*pp_fm)->scale_z=tmp;
    }
    else if(lua_gettop(L)==4)
    {
        (*pp_fm)->scale_x=lua_tointeger(L,2);
        (*pp_fm)->scale_y=lua_tointeger(L,3);
        (*pp_fm)->scale_z=lua_tointeger(L,4);
    }
    
    return 0;
}
