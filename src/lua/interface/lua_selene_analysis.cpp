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

#include <lua_selene.h>

namespace LuaUI
{
    int create_selene_raycounter(lua_State *L)
    {
        Sel::RayCounter *target=new Sel::RayCounter();
        lua_set_metapointer<Sel::RayCounter>(L,"metatable_selene_raycounter",target);

        return 1;
    }


    void selene_create_raycounter_metatable(lua_State *L)
    {
        create_obj_metatable(L,"metatable_selen_raycounter");

        metatable_add_func(L,"hit_count",&LuaUI::selene_raycounter_get_hit_count);
        metatable_add_func(L,"sensor",&LuaUI::selene_raycounter_set_sensor);
    }


    int selene_raycounter_get_hit_count(lua_State *L)
    {
        Sel::RayCounter *counter=lua_get_metapointer<Sel::RayCounter>(L,1);

        int hit_count=counter->compute_hit_count();
        lua_pushinteger(L,hit_count);

        return 1;
    }


    int selene_raycounter_set_sensor(lua_State *L)
    {
        Sel::RayCounter *counter=lua_get_metapointer<Sel::RayCounter>(L,1);
        Sel::Object *sensor=lua_get_metapointer<Sel::Object>(L,2);

        counter->set_sensor(sensor);

        return 0;
    }
}
