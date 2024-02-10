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

#include <lua_selene.h>

namespace LuaUI
{
    int create_selene_target(lua_State *L)
    {
        Sel::OptimTarget *target=new Sel::OptimTarget();
        lua_set_metapointer<Sel::OptimTarget>(L,"metatable_selene_target",target);
        
        return 1;
    }
    
    
    void selene_create_target_metatable(lua_State *L)
    {
        create_obj_metatable(L,"metatable_selene_target");
        
        metatable_add_func(L,"goal",&LuaUI::selene_target_set_operation);
        metatable_add_func(L,"sensor",&LuaUI::selene_target_set_sensor);
        metatable_add_func(L,"weight",&LuaUI::selene_target_set_weight);
    }
    
    
    int selene_target_set_operation(lua_State *L)
    {
        OptimTarget *target=lua_get_metapointer<OptimTarget>(L,1);
        Sel::OptimTarget *sel_target=dynamic_cast<Sel::OptimTarget*>(target);
        
        sel_target->goal=to_goal(lua_tostring(L,2));
        
        if(sel_target->goal==Sel::OptimGoal::TARGET_HIT_COUNT)
        {
            sel_target->target_value=lua_tointeger(L,3);
        }
        
        return 0;
    }
    
    
    int selene_target_set_sensor(lua_State *L)
    {
        OptimTarget *target=lua_get_metapointer<OptimTarget>(L,1);
        Sel::OptimTarget *sel_target=dynamic_cast<Sel::OptimTarget*>(target);
        
        Sel::Frame *frame=lua_get_metapointer<Sel::Frame>(L,2);
        
        sel_target->sensor=dynamic_cast<Sel::Object*>(frame);
        
        return 0;
    }
    
    
    int selene_target_set_weight(lua_State *L)
    {
        OptimTarget *target=lua_get_metapointer<OptimTarget>(L,1);
        Sel::OptimTarget *sel_target=dynamic_cast<Sel::OptimTarget*>(target);
        
        sel_target->weight=lua_tonumber(L,2);
        
        return 0;
    }
    
    
    std::string to_lua(Sel::OptimGoal goal)
    {
        switch(goal)
        {
            case Sel::OptimGoal::MAXIMIZE_HIT_COUNT:
                return "maximize_hit_count";
                
            case Sel::OptimGoal::MINIMIZE_ANGULAR_SPREAD:
                return "minimize_angular_spread";
                
            case Sel::OptimGoal::MINIMIZE_SPATIAL_SPREAD:
                return "minimize_spatial_spread";
                
            case Sel::OptimGoal::TARGET_HIT_COUNT:
                return "target_hit_count";
        }
        
        return "";
    }
    
    
     Sel::OptimGoal to_goal(std::string const &str)
     {
         if(str==to_lua(Sel::OptimGoal::MAXIMIZE_HIT_COUNT))
         {
             return Sel::OptimGoal::MINIMIZE_ANGULAR_SPREAD;
         }
         else if(str==to_lua(Sel::OptimGoal::MINIMIZE_ANGULAR_SPREAD))
         {
             return Sel::OptimGoal::MINIMIZE_ANGULAR_SPREAD;
         }
         else if(str==to_lua(Sel::OptimGoal::MINIMIZE_SPATIAL_SPREAD))
         {
             return Sel::OptimGoal::MINIMIZE_SPATIAL_SPREAD;
         }
         else if(str==to_lua(Sel::OptimGoal::TARGET_HIT_COUNT))
         {
             return Sel::OptimGoal::TARGET_HIT_COUNT;
         }
         
         return Sel::OptimGoal::MINIMIZE_SPATIAL_SPREAD;
     }
}
