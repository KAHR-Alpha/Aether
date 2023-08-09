#include <lua_optim.h>
#include <math_optim.h>

namespace LuaUI
{
    
OptimRule::Operation to_optim_operation(std::string const &str);
OptimRule::Limit to_optim_limit(std::string const &str);

int allocate_optimization_engine(lua_State *L)
{
    lua_allocate_metapointer<OptimEngine>(L,"metatable_optimization_engine");
    
    return 1;
}


void create_optimization_metatable(lua_State *L)
{
    lua_register(L,"Optimizer",&allocate_optimization_engine);
    
    create_obj_metatable(L,"metatable_optimization_engine");
    
    metatable_add_func(L,"add_target",&optimizer_add_target);
    metatable_add_func(L,"optimize",&optimizer_add_variable);
}


int optimizer_add_target(lua_State *L)
{
    OptimEngine *engine=lua_get_metapointer<OptimEngine>(L,1);
    OptimTarget *target=lua_get_metapointer<OptimTarget>(L,2);
    
    engine->add_target(target);
    
    return 0;
}


int optimizer_add_variable(lua_State *L)
{
    OptimEngine *p_engine=lua_get_metapointer<OptimEngine>(L,1);
    
    OptimRule rule;
    
    double *variable=static_cast<double*>(lua_touserdata(L,2));
    
    rule.operation_type=to_optim_operation(lua_tostring(L,3));
    
    if(rule.operation_type==OptimRule::ADD)
    {
        rule.delta_add=lua_tonumber(L,4);
    }
    else rule.delta_grow=lua_tonumber(L,4);
    
    rule.limit_down=lua_tonumber(L,5);
    rule.limit_up=lua_tonumber(L,6);
    rule.limit_type=to_optim_limit(lua_tostring(L,7));
    
    p_engine->register_variable(variable,rule);
    
    return 0;
}


std::string to_lua(OptimRule::Operation operation)
{
    switch(operation)
    {
        case OptimRule::Operation::ADD: return "add";
        case OptimRule::Operation::GROW: return "grow";
    }
    
    return "";
}


std::string to_lua(OptimRule::Limit limit)
{
    switch(limit)
    {
        case OptimRule::Limit::BOTH: return "both";
        case OptimRule::Limit::UP: return "up";
        case OptimRule::Limit::DOWN: return "down";
        case OptimRule::Limit::NONE: return "none";
    }
    
    return "";
}


OptimRule::Operation to_optim_operation(std::string const &str)
{
    if(str==to_lua(OptimRule::Operation::ADD))
    {
        return OptimRule::Operation::ADD;
    }
    else return OptimRule::Operation::GROW;
}


OptimRule::Limit to_optim_limit(std::string const &str)
{
         if(str==to_lua(OptimRule::Limit::BOTH))
    {
        return OptimRule::Limit::BOTH;
    }
    else if(str==to_lua(OptimRule::Limit::UP))
    {
        return OptimRule::Limit::UP;
    }
    else if(str==to_lua(OptimRule::Limit::DOWN))
    {
        return OptimRule::Limit::DOWN;
    }
    else return OptimRule::Limit::NONE;
}

}
