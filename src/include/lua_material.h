/*Copyright 2008-2023 - Loïc Le Cunff

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
#include <material.h>

namespace lua_material
{
    enum class Mode
    {
        LIVE,
        SCRIPT
    };
    
    template<Mode mode>
    Material* get_mat_pointer(lua_State *L)
    {
        if constexpr(mode==Mode::LIVE)
        {
            return lua_get_metapointer<Material>(L,1);
        }
        else
        {
            lua_getglobal(L,"bound_material");
            return reinterpret_cast<Material*>(lua_touserdata(L,-1));
        }
    }
    
    class Loader
    {
        public:
            int (*allocation_function)(lua_State*);
            
            std::vector<std::string> function_names;
            std::vector<int (*)(lua_State*)> functions_live;
            std::vector<int (*)(lua_State*)> functions_script;
            
            Loader();
            virtual ~Loader()=default;
            
            void add_functions(std::string const &name,
                               int (*live_function)(lua_State*),
                               int (*script_function)(lua_State*));
            void create_metatable(lua_State *L);
            void load(Material *material,std::filesystem::path const &script_path);
            void replace_functions(std::string const &name,
                                   int (*live_function)(lua_State*),
                                   int (*script_function)(lua_State*));
            void set_allocation_function(int (*alloc)(lua_State*));
    };
}

#endif // LUA_MATERIAL_H_INCLUDED
