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

#include <lua_sensors.h>
#include <mathUT.h>
#include <sensors.h>

extern const Imdouble Im;

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
    sens.type=to_sensor_type(type);
    
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

Sensor_type to_sensor_type(std::string const &type)
{
         if(type=="box_poynting")  return Sensor_type::BOX_POYNTING;
    else if(type=="box_spectral_poynting") return Sensor_type::BOX_SPECTRAL_POYNTING;
    else if(type=="diff_orders") return Sensor_type::DIFF_ORDERS;
    else if(type=="farfield") return Sensor_type::FARFIELD;
    else if(type=="fieldblock") return Sensor_type::FIELDBLOCK;
    else if(type=="fieldmap") return Sensor_type::FIELDMAP;
    else if(type=="fieldmap2") return Sensor_type::FIELDMAP2;
    else if(type=="fieldpoint") return Sensor_type::FIELDPOINT;
    else if(type=="movie") return Sensor_type::MOVIE;
    else if(type=="planar_spectral_poynting") return Sensor_type::PLANAR_SPECTRAL_POYNTING;
    else
    {
        std::cout<<"Unknown sensor type: "<<type<<std::endl;
        std::cout<<"Press Enter to continue..."<<std::endl;
        std::cin.get();
        
        return Sensor_type::UNKNOWN;
    }
}

std::string from_sensor_type(Sensor_type type)
{
    if(type==Sensor_type::BOX_POYNTING) return "box_poynting";
    else if(type==Sensor_type::BOX_SPECTRAL_POYNTING) return "box_spectral_poynting";
    else if(type==Sensor_type::DIFF_ORDERS) return "diff_orders";
    else if(type==Sensor_type::FARFIELD) return "farfield";
    else if(type==Sensor_type::FIELDBLOCK) return "fieldblock";
    else if(type==Sensor_type::FIELDMAP) return "fieldmap";
    else if(type==Sensor_type::FIELDMAP2) return "fieldmap2";
    else if(type==Sensor_type::FIELDPOINT) return "fieldpoint";
    else if(type==Sensor_type::MOVIE) return "movie";
    else if(type==Sensor_type::PLANAR_SPECTRAL_POYNTING) return "planar_spectral_poynting";
    else
    {
        return "unknown";
    }
}
