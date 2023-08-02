#ifndef LUA_OPTIM_H_INCLUDED
#define LUA_OPTIM_H_INCLUDED

#include <lua_base.h>

namespace LuaUI
{

int allocate_optimization_engine(lua_State *L);
void create_optimization_metatable(lua_State *L);
int optimize(lua_State *L);

};

#endif // LUA_OPTIM_H_INCLUDED
