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

#ifndef LUA_STRUCTURE_H_INCLUDED
#define LUA_STRUCTURE_H_INCLUDED

#include <lua_base.h>

#include <filesystem>

namespace LuaUI
{
int allocate_structure(lua_State *L);
void create_structure_metatable(lua_State *L);
int structure_finalize(lua_State *L);
int structure_print(lua_State *L);
int structure_set_parameter(lua_State *L);

int structure_add_block(lua_State *L);
int structure_add_coating(lua_State *L);
int structure_add_cone(lua_State *L);
int structure_add_cylinder(lua_State *L);
int structure_add_ellipsoid(lua_State *L);
int structure_add_layer(lua_State *L);
int structure_add_lua_def(lua_State *L);
int structure_add_mesh(lua_State *L);
int structure_add_sin_layer(lua_State *L);
int structure_add_sphere(lua_State *L);
int structure_add_vect_block(lua_State *L);
int structure_add_vect_tri(lua_State *L);
int structure_declare_parameter(lua_State *L);
int structure_default_material(lua_State *L);

int structure_set_flip(lua_State *L);
int structure_set_loop(lua_State *L);

int random_packing(lua_State *L);

[[deprecated]]
int gen_empty_structure(lua_State *L);

[[deprecated]]
int gen_multilayer(lua_State *L);

[[deprecated]]
int gen_simple_substrate(lua_State *L);

[[deprecated]]
int gen_slab(lua_State *L);
}

std::string ageom_to_lua(std::filesystem::path const &script);

#endif // LUA_STRUCTURE_H_INCLUDED
