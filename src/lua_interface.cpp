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

#include <index_utils.h>
#include <lua_interface.h>
#include <material.h>
#include <multilayers.h>
#include <planar_wgd.h>


extern std::ofstream plog;

//###################################
//       Dielec planar waveguide
//###################################

Dielec_pwgd_mode::Dielec_pwgd_mode()
    :n1(1.0), n2(3.42), n3(1.5),
     lambda(1500e-9), h(500e-9)
{
}

void Dielec_pwgd_mode::process()
{
    Slab_guide slb(h,n1,n2,n3);
    
    slb.compute_guiding_char(lambda);
    slb.show_guiding_char();
}

int dielec_pwgd_mode_set_guide_index(lua_State *L)
{
    Dielec_pwgd_mode **pp_dpw=reinterpret_cast<Dielec_pwgd_mode**>(lua_touserdata(L,1));
    
    (*pp_dpw)->n2=lua_tonumber(L,2);
    
    return 1;
}

int dielec_pwgd_mode_set_lambda(lua_State *L)
{
    Dielec_pwgd_mode **pp_dpw=reinterpret_cast<Dielec_pwgd_mode**>(lua_touserdata(L,1));
    
    (*pp_dpw)->lambda=lua_tonumber(L,2);
    
    return 1;
}

int dielec_pwgd_mode_set_sub_index(lua_State *L)
{
    Dielec_pwgd_mode **pp_dpw=reinterpret_cast<Dielec_pwgd_mode**>(lua_touserdata(L,1));
    
    (*pp_dpw)->n3=lua_tonumber(L,2);
    
    return 1;
}

int dielec_pwgd_mode_set_sup_index(lua_State *L)
{
    Dielec_pwgd_mode **pp_dpw=reinterpret_cast<Dielec_pwgd_mode**>(lua_touserdata(L,1));
    
    (*pp_dpw)->n1=lua_tonumber(L,2);
    
    return 1;
}

int dielec_pwgd_mode_set_thickness(lua_State *L)
{
    Dielec_pwgd_mode **pp_dpw=reinterpret_cast<Dielec_pwgd_mode**>(lua_touserdata(L,1));
    
    (*pp_dpw)->h=lua_tonumber(L,2);
    
    return 1;
}

//####################
//    Index fit
//####################

Index_fit_mode::Index_fit_mode()
    :fname(""), output("index_fit"),
     Ndrude(0), Nlorentz(0), Ncp(0)
{
}

void Index_fit_mode::process()
{
    index_fit(fname,Ndrude,Nlorentz,Ncp);
}

int index_fit_mode_set_file(lua_State *L)
{
    Index_fit_mode **pp_if=reinterpret_cast<Index_fit_mode**>(lua_touserdata(L,1));
    
    (*pp_if)->fname=lua_tostring(L,2);
    
    return 1;
}

int index_fit_mode_set_name(lua_State *L)
{
    Index_fit_mode **pp_if=reinterpret_cast<Index_fit_mode**>(lua_touserdata(L,1));
    
    (*pp_if)->output=lua_tostring(L,2);
    
    return 1;
}

int index_fit_mode_set_Ndrude(lua_State *L)
{
    Index_fit_mode **pp_if=reinterpret_cast<Index_fit_mode**>(lua_touserdata(L,1));
    
    (*pp_if)->Ndrude=lua_tointeger(L,2);
    
    return 1;
}

int index_fit_mode_set_Nlorentz(lua_State *L)
{
    Index_fit_mode **pp_if=reinterpret_cast<Index_fit_mode**>(lua_touserdata(L,1));
    
    (*pp_if)->Nlorentz=lua_tointeger(L,2);
    
    return 1;
}

int index_fit_mode_set_Ncp(lua_State *L)
{
    Index_fit_mode **pp_if=reinterpret_cast<Index_fit_mode**>(lua_touserdata(L,1));
    
    (*pp_if)->Ncp=lua_tointeger(L,2);
    
    return 1;
}

//###############
//     Mie
//###############

int mie_mode_set_env_index(lua_State *L)
{
    Mie_mode **pp_mie=reinterpret_cast<Mie_mode**>(lua_touserdata(L,1));
    
    (*pp_mie)->env_index=lua_tonumber(L,2);
    
    return 0;
}

int mie_mode_set_material(lua_State *L)
{
    Mie_mode **pp_mie=reinterpret_cast<Mie_mode**>(lua_touserdata(L,1));
    
    (*pp_mie)->part_material=lua_tostring(L,2);
    
    return 0;
}

int mie_mode_set_radius(lua_State *L)
{
    Mie_mode **pp_mie=reinterpret_cast<Mie_mode**>(lua_touserdata(L,1));
    
    (*pp_mie)->radius=lua_tonumber(L,2);
    
    return 0;
}

