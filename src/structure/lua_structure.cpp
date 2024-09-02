/*Copyright 2008-2024 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include <filehdl.h>
#include <logger.h>
#include <lua_structure.h>
#include <phys_tools.h>
#include <string_tools.h>
#include <structure.h>

#include <fstream>
#include <sstream>



namespace LuaUI
{
    int spec_struct=0;

[[deprecated]]
int gen_empty_structure(lua_State *L)
{
    std::stringstream fname;
    
    fname<<"empty_structure_";
    fname<<spec_struct;
    fname<<".lua";
    
    std::ofstream file(PathManager::to_temporary_path(fname.str()),std::ios::out|std::ios::trunc);
    
    int Nx=lua_tointeger(L,1);
    int Ny=lua_tointeger(L,2);
    int Nz=lua_tointeger(L,3);
    
    double Dx=lua_tonumber(L,4);
    double Dy=lua_tonumber(L,5);
    double Dz=lua_tonumber(L,6);
    
    file<<"Nx="<<Nx<<std::endl;
    file<<"Ny="<<Ny<<std::endl;
    file<<"Nz="<<Nz<<std::endl<<std::endl;
    
    file<<"Dx="<<Dx<<std::endl;
    file<<"Dy="<<Dy<<std::endl;
    file<<"Dz="<<Dz<<std::endl<<std::endl;
    
    file<<"set_full(0)"<<std::endl;
    file.close();
    
    lua_pushstring(L,fname.str().c_str());
    
    spec_struct++;
    
    return 1;
}

[[deprecated]]
int gen_multilayer(lua_State *L)
{
    int i;
    
    std::stringstream fname;
    
    fname<<"multilayer_";
    fname<<spec_struct;
    fname<<".lua";
    
    std::ofstream file(PathManager::to_temporary_path(fname.str()),std::ios::out|std::ios::trunc);
    
    int Nx=lua_tointeger(L,1);
    int Ny=lua_tointeger(L,2);
    int Nz=lua_tointeger(L,3);
    
    double Dx=lua_tonumber(L,4);
    double Dy=lua_tonumber(L,5);
    double Dz=lua_tonumber(L,6);
    
    int N=lua_gettop(L)-6;
    
    std::vector<int> h_sum;
    
    h_sum.push_back(0);
    for(i=0;i<N;i++)
    {
        int h=lua_tointeger(L,i+7);
        h_sum.push_back(h_sum[i]+h);
    }
    
    int z0=(Nz-h_sum[N])/2;
    
    for(i=0;i<=N;i++) h_sum[i]+=z0;
    
    file<<"Nx="<<Nx<<std::endl;
    file<<"Ny="<<Ny<<std::endl;
    file<<"Nz="<<Nz<<std::endl<<std::endl;
    
    file<<"Dx="<<Dx<<std::endl;
    file<<"Dy="<<Dy<<std::endl;
    file<<"Dz="<<Dz<<std::endl<<std::endl;
    
    file<<"set_full(0)"<<std::endl;
    file<<"add_layer_g(\"Z\",0,"<<Nz/2<<",1)"<<std::endl;
    for(i=0;i<N;i++)
        file<<"add_layer_g(\"Z\","<<h_sum[i]<<","<<h_sum[i+1]<<","<<2+i<<")"<<std::endl;
    file.close();
    
    lua_pushstring(L,fname.str().c_str());
    
    spec_struct++;
    
    return 1;
}

[[deprecated]]
int gen_simple_substrate(lua_State *L)
{
    std::stringstream fname;
    
    fname<<"simple_substrate_";
    fname<<spec_struct;
    fname<<".lua";
    
    std::ofstream file(PathManager::to_temporary_path(fname.str()),std::ios::out|std::ios::trunc);
    
    int Nx=lua_tointeger(L,1);
    int Ny=lua_tointeger(L,2);
    int Nz=lua_tointeger(L,3);
    
    double Dx=lua_tonumber(L,4);
    double Dy=lua_tonumber(L,5);
    double Dz=lua_tonumber(L,6);
    
    file<<"Nx="<<Nx<<std::endl;
    file<<"Ny="<<Ny<<std::endl;
    file<<"Nz="<<Nz<<std::endl<<std::endl;
    
    file<<"Dx="<<Dx<<std::endl;
    file<<"Dy="<<Dy<<std::endl;
    file<<"Dz="<<Dz<<std::endl<<std::endl;
    
    file<<"set_full(0)"<<std::endl;
    file<<"add_layer_g(\"Z\",0,"<<Nz/2<<",1)"<<std::endl;
    file.close();
    
    lua_pushstring(L,fname.str().c_str());
    
    spec_struct++;
    
    return 1;
}

[[deprecated]]
int gen_slab(lua_State *L)
{
    std::stringstream fname;
    
    fname<<"slab_";
    fname<<spec_struct;
    fname<<".lua";
    
    std::ofstream file(PathManager::to_temporary_path(fname.str()),std::ios::out|std::ios::trunc);
    
    int Nx=lua_tointeger(L,1);
    int Ny=lua_tointeger(L,2);
    int Nz=lua_tointeger(L,3);
    
    double Dx=lua_tonumber(L,4);
    double Dy=lua_tonumber(L,5);
    double Dz=lua_tonumber(L,6);
    
    int h=lua_tointeger(L,7);
    
    file<<"Nx="<<Nx<<std::endl;
    file<<"Ny="<<Ny<<std::endl;
    file<<"Nz="<<Nz<<std::endl<<std::endl;
    
    file<<"Dx="<<Dx<<std::endl;
    file<<"Dy="<<Dy<<std::endl;
    file<<"Dz="<<Dz<<std::endl<<std::endl;
    
    file<<"set_full(0)"<<std::endl;
    file<<"add_layer_g(\"Z\",0,"<<Nz/2<<",1)"<<std::endl;
    file<<"add_layer_g(\"Z\","<<Nz/2-h/2<<","<<Nz/2+h-h/2<<",2)"<<std::endl;
    file.close();
    
    lua_pushstring(L,fname.str().c_str());
    
    spec_struct++;
    
    return 1;
}

//###############
//   Structure
//###############

// Bindings allocation

int allocate_structure(lua_State *L)
{
    lua_getglobal(L,"lua_caller_path");
    std::filesystem::path *caller_path=reinterpret_cast<std::filesystem::path*>(lua_touserdata(L,-1));
    
    std::filesystem::path script_path=lua_tostring(L,1);
    script_path=PathManager::locate_file(script_path,*caller_path);
    
    lua_allocate_metapointer<Structure>(L,"metatable_structure",script_path);
    
    return 1;
}

void create_structure_metatable(lua_State *L)
{
    create_obj_metatable(L,"metatable_structure");
    
    metatable_add_func(L,"finalize",structure_finalize);
    metatable_add_func(L,"parameter",structure_set_parameter);
    metatable_add_func(L,"print",structure_print);
}

// Member functions

int structure_finalize(lua_State *L)
{
    Structure *p_struct=lua_get_metapointer<Structure>(L,1);
    
    p_struct->finalize();
    
    return 0;
}

int structure_print(lua_State *L)
{
    Structure *p_struct=lua_get_metapointer<Structure>(L,1);
    
    std::filesystem::path path=lua_tostring(L,2);
    
    double Dx,Dy,Dz;
    
    if(lua_gettop(L)==5)
    {
        Dx=lua_tonumber(L,3);
        Dy=lua_tonumber(L,4);
        Dz=lua_tonumber(L,5);
    }
    else Dx=Dy=Dz=lua_tonumber(L,3);
    
    p_struct->print(path,Dx,Dy,Dz);
    
    return 0;
}

int structure_set_parameter(lua_State *L)
{
    Structure *p_struct=lua_get_metapointer<Structure>(L,1);
    
    std::string parameter_name=lua_tostring(L,2);
    double parameter_value=lua_tonumber(L,3);
    
    bool found=false;
    std::size_t k=vector_locate(found,p_struct->parameter_name,parameter_name);
    
    if(found) p_struct->parameter_value[k]=parameter_value;
    else
    {
        p_struct->parameter_name.push_back(parameter_name);
        p_struct->parameter_value.push_back(parameter_value);
    }
    
    return 0;
}

// Geometric operations

Structure* get_structure_pointer(lua_State *L)
{
    lua_getglobal(L,"lua_calling_class");
    return reinterpret_cast<Structure*>(lua_touserdata(L,-1));
}

int structure_add_block(lua_State *L)
{
    Structure *p_struct=get_structure_pointer(L);
    
    double x1=lua_tonumber(L,1);
    double x2=lua_tonumber(L,2);
    double y1=lua_tonumber(L,3);
    double y2=lua_tonumber(L,4);
    double z1=lua_tonumber(L,5);
    double z2=lua_tonumber(L,6);
    int index=lua_tointeger(L,7);
    
    Plog::print("Adding a block with index ", index, "\n"
             , "     from X( ", add_unit_u(x1), " , ", add_unit_u(x2), " ) ", "\n"
             , "     from Y( ", add_unit_u(y1), " , ", add_unit_u(y2), " ) ", "\n"
             , "     from Z( ", add_unit_u(z1), " , ", add_unit_u(z2), " ) ", "\n");
    
    p_struct->add_operation(new Add_Block(x1,x2,y1,y2,z1,z2,index));
    
    return 0;
}

int structure_add_coating(lua_State *L)
{
    Structure *p_struct=get_structure_pointer(L);
    
    AngleRad theta=Degree(lua_tonumber(L,1));
    AngleRad phi=Degree(lua_tonumber(L,2));
    double thickness=lua_tonumber(L,3);
    double delta=lua_tonumber(L,4);
    int origin_index=lua_tointeger(L,5);
    int index=lua_tointeger(L,6);
    
    Plog::print("Adding a coating with index ", index, " from index ", origin_index, "\n"
             , "     deposited at an incidence of ", theta.degree(), " and an azimuth of ", phi.degree(), "\n"
             , "     with a thickness of ", add_unit_u(thickness), " and a delta check of ", add_unit_u(delta), "\n");
    
    p_struct->add_operation(new Add_Coating(theta,phi,thickness,delta,origin_index,index));
    
    return 0;
}

int structure_add_cone(lua_State *L)
{
    Structure *p_struct=get_structure_pointer(L);
    
    double x1=lua_tonumber(L,1);
    double y1=lua_tonumber(L,2);
    double z1=lua_tonumber(L,3);
    double x2=lua_tonumber(L,4);
    double y2=lua_tonumber(L,5);
    double z2=lua_tonumber(L,6);
    double r=lua_tonumber(L,7);
    int index=lua_tointeger(L,8);
    
    Plog::print("Adding cone with index ", index, "\n"
             , "     from ( ", x1, " , ", y1, " , ", z1, " ) ", "\n"
             , "     to ( ", x1+x2, " , ", y1+y2, " , ", z1+z2, " ) ", "\n"
             , "     with radius ", r, "\n");
    
    p_struct->add_operation(new Add_Cone(x1,y1,z1,x1+x2,y1+y2,z1+z2,r,index));
    
    return 0;
}


    int structure_add_conf_coating(lua_State *L)
    {
        Structure *p_struct=get_structure_pointer(L);

        double thickness = lua_tonumber(L,1);
        int origin_index = lua_tointeger(L,2);
        double delta = lua_tonumber(L,3);
        int index = lua_tointeger(L,4);

        Plog::print("Adding a conformal coating with index ", index, " from index ", origin_index, "\n",
                    "     with a thickness of ", add_unit_u(thickness), " and a delta check of ", add_unit_u(delta), "\n");

        p_struct->add_operation(new Add_Conf_Coating(thickness, origin_index, delta, index));

        return 0;
    }


int structure_add_cylinder(lua_State *L)
{
    Structure *p_struct=get_structure_pointer(L);
    
    double x1=lua_tonumber(L,1);
    double y1=lua_tonumber(L,2);
    double z1=lua_tonumber(L,3);
    double x2=lua_tonumber(L,4);
    double y2=lua_tonumber(L,5);
    double z2=lua_tonumber(L,6);
    double r=lua_tonumber(L,7);
    int index=lua_tointeger(L,8);
    
    Plog::print("Adding cylinder with index ", index, "\n"
             , "     from ( ", x1, " , ", y1, " , ", z1, " ) ", "\n"
             , "     to ( ", x1+x2, " , ", y1+y2, " , ", z1+z2, " ) ", "\n"
             , "     with radius ", r, "\n");
    
    p_struct->add_operation(new Add_Cylinder(x1,y1,z1,x1+x2,y1+y2,z1+z2,r,index));
    
    return 0;
}

int structure_add_ellipsoid(lua_State *L)
{
    Structure *p_struct=get_structure_pointer(L);
    
    double x=lua_tonumber(L,1);
    double y=lua_tonumber(L,2);
    double z=lua_tonumber(L,3);
    double rx=lua_tonumber(L,4);
    double ry=lua_tonumber(L,5);
    double rz=lua_tonumber(L,6);
    int index=lua_tointeger(L,7);
    
    Plog::print("Adding an ellipsoid with index ", index, "\n"
             , "     at ( ", add_unit_u(x), " , ", add_unit_u(y), " , ", add_unit_u(z), " ) with arm lengthes ("
             , add_unit_u(rx), " , ", add_unit_u(ry), " , ", add_unit_u(rz), ")", "\n");
    
    p_struct->add_operation(new Add_Ellipsoid(x,y,z,rx,ry,rz,index));
    
    return 0;
}

int structure_add_layer(lua_State *L)
{
    Structure *p_struct=get_structure_pointer(L);
    
    std::string type_str=lua_tostring(L,1);
    double z1=lua_tonumber(L,2);
    double z2=lua_tonumber(L,3);
    int index=lua_tointeger(L,4);
    
    int type=2;
    
    if(type_str=="X" || type_str=="x") type=0;
    else if(type_str=="Y" || type_str=="y") type=1;
    
    Plog::print("Adding layer with index ", index, "\n"
             , "     along ", type_str, "\n"
             , "     between ", add_unit_u(z1), " and ", add_unit_u(z2), "\n");
    
    p_struct->add_operation(new Add_Layer(type,z1,z2,index));
    
    return 0;
}

int structure_add_lua_def(lua_State *L)
{
    int i;
    
    std::string fname;
    lua_State *mom_state;
    std::vector<lua_tools::lua_type*> parameters;
    int mat_index;
    
    int N=lua_gettop(L);
    
    fname=lua_tostring(L,1);
    for(i=2;i<N;i++)
    {
             if(lua_isinteger(L,i)) parameters.push_back(new lua_tools::lua_int);
        else if(lua_isnumber(L,i)) parameters.push_back(new lua_tools::lua_double);
        else if(lua_isstring(L,i)) parameters.push_back(new lua_tools::lua_string);
        
        parameters.back()->set_value(L,i);
    }
    mat_index=lua_tointeger(L,N);
    
    Structure *p_struct=get_structure_pointer(L);
    
    lua_getglobal(L,"lua_mother_state");
    mom_state=reinterpret_cast<lua_State*>(lua_touserdata(L,-1));
    
    p_struct->add_operation(new Add_Lua_Def(mom_state,fname,parameters,mat_index));
    
    return 0;
}

int structure_add_mesh(lua_State *L)
{
    Structure *p_struct=get_structure_pointer(L);
    
    double x=lua_tonumber(L,1);
    double y=lua_tonumber(L,2);
    double z=lua_tonumber(L,3);
    double s=lua_tonumber(L,4);
    std::string fname=lua_tostring(L,5);
    int index=lua_tointeger(L,6);
    
    std::filesystem::path fpath=fname;
    
    lua_getglobal(L,"lua_caller_path");
    std::filesystem::path *caller_path=reinterpret_cast<std::filesystem::path*>(lua_touserdata(L,-1));
    
    fpath=PathManager::locate_file(fpath,*caller_path);
    
    Plog::print("Adding a mesh from the file ", fpath, " with index ", index, "\n"
             , "     at ( ", add_unit_u(x), " , ", add_unit_u(y), " , ", add_unit_u(z), " ) with scale "
             , s, "\n");
    
    p_struct->add_operation(new Add_Mesh(x,y,z,s,fpath,index));
    
    return 0;
}

int structure_add_sin_layer(lua_State *L)
{
    Structure *p_struct=get_structure_pointer(L);
    
    double z1=lua_tonumber(L,1);
    double a1=lua_tonumber(L,2);
    
    double z2=lua_tonumber(L,3);
    double a2=lua_tonumber(L,4);
    
    double px=lua_tonumber(L,5);
    double phi_x=lua_tonumber(L,6);
    
    double py=lua_tonumber(L,7);
    double phi_y=lua_tonumber(L,8);
    
    int index=lua_tointeger(L,9);
    
    Plog::print("Adding a sinusoidal with index ", index, "\n"
             , "     between z=", add_unit_u(z1), " and z=", add_unit_u(z2), "\n"
             , "     with amplitudes of ", add_unit_u(a1), " and ", add_unit_u(a2), "\n"
             , "     with periodicities of ", add_unit_u(px), " and ", add_unit_u(py), "\n"
             , "     with phases of ", 180.0/Pi*phi_x, " and ", 180.0/Pi*phi_y, "\n");
    
    p_struct->add_operation(new Add_Sin_Layer(z1,a1,z2,a2,px,phi_x,py,phi_y,index));
    
    return 0;
}

int structure_add_sphere(lua_State *L)
{
    Structure *p_struct=get_structure_pointer(L);
    
    double x=lua_tonumber(L,1);
    double y=lua_tonumber(L,2);
    double z=lua_tonumber(L,3);
    double r=lua_tonumber(L,4);
    int index=lua_tointeger(L,5);
    
    Plog::print("Adding a sphere with index ", index, "\n"
             , "     at ( ", add_unit_u(x), " , ", add_unit_u(y), " , ", add_unit_u(z), " ) with radius "
             , add_unit_u(r), "\n");
    
    p_struct->add_operation(new Add_Sphere(x,y,z,r,index));
    
    return 0;
}

int structure_add_vect_block(lua_State *L)
{
    Structure *p_struct=get_structure_pointer(L);

    double xO=lua_tonumber(L,1);
    double yO=lua_tonumber(L,2);
    double zO=lua_tonumber(L,3);
    
    double xA=lua_tonumber(L,4);
    double yA=lua_tonumber(L,5);
    double zA=lua_tonumber(L,6);
    
    double xB=lua_tonumber(L,7);
    double yB=lua_tonumber(L,8);
    double zB=lua_tonumber(L,9);
    
    double xC=lua_tonumber(L,10);
    double yC=lua_tonumber(L,11);
    double zC=lua_tonumber(L,12);
    int index=lua_tointeger(L,13);
    
    Plog::print("Adding a vector block with index ", index, "\n");
    Plog::print("    with O( ", xO, " , ", yO, " , ", zO, " )", "\n");
    Plog::print("    with A( ", xA, " , ", yA, " , ", zA, " )", "\n");
    Plog::print("    with B( ", xB, " , ", yB, " , ", zB, " )", "\n");
    Plog::print("    with C( ", xC, " , ", yC, " , ", zC, " )", "\n");
    
    p_struct->add_operation(new Add_Vect_Block(xO,yO,zO,xA,yA,zA,xB,yB,zB,xC,yC,zC,index));
    
    return 0;
}

int structure_add_vect_tri(lua_State *L)
{
    Structure *p_struct=get_structure_pointer(L);

    double xO=lua_tonumber(L,1);
    double yO=lua_tonumber(L,2);
    double zO=lua_tonumber(L,3);
    
    double xA=lua_tonumber(L,4);
    double yA=lua_tonumber(L,5);
    double zA=lua_tonumber(L,6);
    
    double xB=lua_tonumber(L,7);
    double yB=lua_tonumber(L,8);
    double zB=lua_tonumber(L,9);
    
    double xC=lua_tonumber(L,10);
    double yC=lua_tonumber(L,11);
    double zC=lua_tonumber(L,12);
    
    double alp=lua_tonumber(L,13);
    int index=lua_tointeger(L,14);
    
    Plog::print("Adding a vector triangle with index ", index, "\n");
    Plog::print("    with O( ", xO, " , ", yO, " , ", zO, " )", "\n");
    Plog::print("    with A( ", xA, " , ", yA, " , ", zA, " )", "\n");
    Plog::print("    with B( ", xB, " , ", yB, " , ", zB, " )", "\n");
    Plog::print("    with C( ", xC, " , ", yC, " , ", zC, " )", "\n");
    
    p_struct->add_operation(new Add_Vect_Tri(xO,yO,zO,xA,yA,zA,xB,yB,zB,xC,yC,zC,alp,index));
    
    return 0;
}

int structure_declare_parameter(lua_State *L)
{
    std::string parameter_name=lua_tostring(L,1);
    double parameter_default_value=lua_tonumber(L,2);
    
    Structure *p_struct=get_structure_pointer(L);
    
    bool found=false;
    
    // Locating in the script parameters
    
    std::size_t k=vector_locate(found,p_struct->parameter_name,parameter_name);
    
    if(!found)
    {
        p_struct->parameter_name.push_back(parameter_name);
        p_struct->parameter_value.push_back(parameter_default_value);
    }
    
    // Locating in the input parameters
    
    k=vector_locate(found,p_struct->parameter_name,parameter_name);
    
    if(found) lua_pushnumber(L,p_struct->parameter_value[k]);
    else lua_pushnumber(L,parameter_default_value);
    
    lua_setglobal(L,parameter_name.c_str());
    
    return 1;
}

int structure_default_material(lua_State *L)
{
    Structure *p_struct=get_structure_pointer(L);

    int index=lua_tointeger(L,1);
    Plog::print("Filling with index ", index, "\n");
    
    p_struct->set_default_material(index);
    
    return 0;
}

int structure_set_flip(lua_State *L)
{
    Structure *p_struct=get_structure_pointer(L);
    
    p_struct->set_flip(lua_tointeger(L,1),
                       lua_tointeger(L,2),
                       lua_tointeger(L,3));
    
    return 0;
}

int structure_set_loop(lua_State *L)
{
    Structure *p_struct=get_structure_pointer(L);
    
    p_struct->set_loop(lua_tointeger(L,1),
                       lua_tointeger(L,2),
                       lua_tointeger(L,3));
    
    return 0;
}

//   Helper functions

int random_packing(lua_State *L)
{
    double x_max=lua_tonumber(L,1);
    double y_max=lua_tonumber(L,2);
    double r=lua_tonumber(L,3);
    int Npart=lua_tointeger(L,4);
        
    int gen_seed=std::time(0);
    
    if(lua_gettop(L)==5)
        gen_seed=lua_tointeger(L,5);
    
    std::vector<SeedParticle> particles(Npart);
    
    random_packing(particles,x_max,y_max,r,Npart,gen_seed);
    
    lua_newtable(L);
    
    for(int i=0;i<Npart;i++)
    {
        lua_pushnumber(L,particles[i].x);
        lua_rawseti(L,-2,i+1);
    }
    
    lua_newtable(L);
    
    for(int i=0;i<Npart;i++)
    {
        lua_pushnumber(L,particles[i].y);
        lua_rawseti(L,-2,i+1);
    }
    
    return 2;
}

}
