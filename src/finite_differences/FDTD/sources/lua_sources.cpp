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

#include <enum_constants.h>
#include <lua_sources.h>
#include <mathUT.h>
#include <sources.h>
#include <string_tools.h>

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
