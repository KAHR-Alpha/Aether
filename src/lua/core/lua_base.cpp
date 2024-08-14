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

#include <logger.h>
#include <lua_base.h>
#include <mathUT.h>

int cpp_to_lua_pow(lua_State *L)
{
    double val=lua_tonumber(L,1);
    double pw=lua_tonumber(L,2);
    
    double r=std::pow(val,pw);
    lua_pushnumber(L,r);
    
    return 1;
}

void create_obj_metatable(lua_State *L,std::string meta_name)
{
    luaL_newmetatable(L,meta_name.c_str());
    lua_pushvalue(L,-1);
    lua_setfield(L,-2,"__index");
}

int declare_parameter_cli(lua_State *L)
{
    std::string param_name=lua_tostring(L,1);
    std::string param_def_val=lua_tostring(L,2);
    
    lua_getglobal(L,param_name.c_str());
    if(lua_isnil(L,-1))
    {
        Plog::print("Undefined parameter ", param_name, " , defining it as ",
                    param_name, "=", param_def_val, "\n");
        
        lua_pushnumber(L,std::stod(param_def_val));
        lua_setglobal(L,param_name.c_str());
    }
    
    return 1;
}

// Copied from lua 5.4.4 source code
int msghandler (lua_State *L)
{
    const char *msg = lua_tostring(L, 1);
    if (msg == NULL)   /* is error object not a string? */
    {
        if (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
                lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
            return 1;  /* that is the message */
        else
            msg = lua_pushfstring(L, "(error object is a %s value)",
                                  luaL_typename(L, 1));
    }
    luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
    return 1;  /* return the traceback */
}

int docall(lua_State *L, int narg, int nres)
{
    int status;
    int base = lua_gettop(L) - narg;  /* function index */
    lua_pushcfunction(L, msghandler);  /* push message handler */
    lua_insert(L, base);  /* put it under function and args */
    status = lua_pcall(L, narg, nres, base);
    lua_remove(L, base);  /* remove message handler from the stack */
    return status;
}

void metatable_add_func(lua_State *L,std::string fname,int (*f)(lua_State*))
{
    lua_pushcfunction(L,f);
    lua_setfield(L,-2,fname.c_str());
}

int nearest_integer(lua_State *L)
{
    double rd=lua_tonumber(L,1);
    int r=static_cast<int>(rd+0.5);
    lua_pushinteger(L,r);
    
    return 1;
}

int null_lua(lua_State *L) { return 1; }

namespace lua_tools
{
    class LuaFunctor
    {
        public:
            lua_State *L;
            std::string fname;
            
            double operator() (double x)
            {
                lua_getglobal(L,fname.c_str());
                
                lua_pushvalue(L,-1);
                lua_pushnumber(L,x);
                
                lua_call(L,1,1);
                
                double r=lua_tonumber(L,-1);
                
                lua_pop(L,1);
                
                return r;
            }
    };
    
    int lua_adaptive_trapeze_integral(lua_State *L)
    {
        std::string fname=lua_tostring(L,1);
        double x_min=lua_tonumber(L,2);
        double x_max=lua_tonumber(L,3);
        int Np=lua_tointeger(L,4);
        
        LuaFunctor f;
        f.L=L;
        f.fname=fname;
        
        double r=adaptive_trapeze_integral(Np,f,x_min,x_max);
        
        lua_pushnumber(L,r);
        
        return 1;
    }
    
    void extract_vector(std::vector<double> &vec,lua_State *L,int index)
    {
        vec.clear();
        
        lua_pushnil(L);
        
        while(lua_next(L,index)!=0)
        {
            vec.push_back(lua_tonumber(L,-1));
            lua_pop(L,1);
        }
    }
}
