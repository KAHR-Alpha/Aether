#ifndef LUA_OPTIM_H_INCLUDED
#define LUA_OPTIM_H_INCLUDED

#include <lua_base.h>
#include <math_optim.h>

namespace LuaUI
{

int allocate_optimization_engine(lua_State *L);
void create_optimization_metatable(lua_State *L);
int optimizer_add_target(lua_State *L);
int optimizer_add_variable(lua_State *L);
int optimizer_set_max_failures(lua_State *L);

std::string to_lua(OptimRule::Operation operation);
std::string to_lua(OptimRule::Limit limit);

}

#endif // LUA_OPTIM_H_INCLUDED
