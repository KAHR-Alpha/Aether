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
