#include <lua_optim.h>
#include <math_optim.h>

namespace LuaUI
{

int allocate_optimization_engine(lua_State *L)
{
    lua_allocate_metapointer<OptimEngine>(L,"metatable_optimization_engine");
    
    return 1;
}


void create_optimization_metatable(lua_State *L)
{
    lua_pushinteger(L,OptimRule::Operation::ADD);
    lua_setglobal(L,"OPTIM_ADD");
    
    lua_pushinteger(L,OptimRule::Operation::GROW);
    lua_setglobal(L,"OPTIM_GROW");
    
    lua_pushinteger(L,OptimRule::Limit::UP);
    lua_setglobal(L,"OPTIM_LIMIT_UP");
    
    lua_pushinteger(L,OptimRule::Limit::DOWN);
    lua_setglobal(L,"OPTIM_LIMIT_DOWN");
    
    lua_pushinteger(L,OptimRule::Limit::BOTH);
    lua_setglobal(L,"OPTIM_LIMIT_BOTH");
    
    lua_pushinteger(L,OptimRule::Limit::NONE);
    lua_setglobal(L,"OPTIM_LIMIT_NONE");
    
    lua_register(L,"Optimizer",&allocate_optimization_engine);
    
    create_obj_metatable(L,"metatable_optimization_engine");
    
    metatable_add_func(L,"optimize",&optimize);
}


int optimize(lua_State *L)
{
    OptimEngine *p_engine=lua_get_metapointer<OptimEngine>(L,1);
    
    OptimRule rule;
    
    double *variable=static_cast<double*>(lua_touserdata(L,2));
    
    rule.operation_type=static_cast<OptimRule::Operation>(lua_tointeger(L,3));
    
    if(rule.operation_type==OptimRule::ADD)
    {
        rule.delta_add=lua_tonumber(L,4);
    }
    else rule.delta_grow=lua_tonumber(L,4);
    
    rule.limit_down=lua_tonumber(L,5);
    rule.limit_up=lua_tonumber(L,6);
    rule.limit_type=static_cast<OptimRule::Limit>(lua_tointeger(L,7));
    
    p_engine->register_variable(variable,rule);
    
    return 0;
}

}
