/*Copyright 2008-2023 - Lo�c Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#ifndef LUA_MATERIAL_H_INCLUDED
#define LUA_MATERIAL_H_INCLUDED

#include <lua_base.h>

namespace lua_material
{
    void create_metatable(lua_State *L);
    
    int allocate(lua_State *L);
    int set_index(lua_State *L);
    int set_script(lua_State *L);
    int set_name(lua_State *L);
    
    int add_cauchy(lua_State *L);
    int add_crit_point(lua_State *L);
    int add_data_epsilon(lua_State *L);
    int add_data_index(lua_State *L);
    int add_debye(lua_State *L);
    int add_drude(lua_State *L);
    int add_lorentz(lua_State *L);
    int add_sellmeier(lua_State *L);
    int description(lua_State *L);
    int epsilon_infinity(lua_State *L);
    int set_name(lua_State *L);
    int validity_range(lua_State *L);
}

#endif // LUA_MATERIAL_H_INCLUDED