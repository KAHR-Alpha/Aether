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

#ifndef LUA_BASE_H_INCLUDED
#define LUA_BASE_H_INCLUDED

#include <logger.h>
#include <lua.hpp>

#include <iostream>
#include <tuple>
#include <utility>
#include <vector>

int declare_parameter_cli(lua_State *L);
int docall(lua_State *L, int narg, int nres);
void create_obj_metatable(lua_State *L,std::string meta_name);
void metatable_add_func(lua_State *L,std::string fname,int (*f)(lua_State*));
int cpp_to_lua_pow(lua_State *L);
int nearest_integer(lua_State *L);
int null_lua(lua_State *L);

namespace lua_tools
{
    int lua_adaptive_trapeze_integral(lua_State *L);
    void extract_vector(std::vector<double> &vec,lua_State *L,int index);
    
    class lua_type
    {
        public:
            virtual void push_value(lua_State *L) {}
            virtual void set_value(lua_State *L,int index) {}
            virtual void show() {}
    };
    
    class lua_double: public lua_type
    {
        public:
            double value;
            
            lua_double()
                :value(0) 
            {}
            
            lua_double(double value_)
                :value(value_)
            {}
            
            void push_value(lua_State *L)
            {
                lua_pushnumber(L,value);
            }
            
            void set_value(lua_State *L,int index)
            {
                value=lua_tonumber(L,index);
            }
            
            void show()
            {
                Plog::print(value, "\n");
            }
    };
    
    class lua_int: public lua_type
    {
        public:
            int value;
            
            lua_int()
                :value(0) 
            {}
            
            lua_int(int value_)
                :value(value_)
            {}
            
            void push_value(lua_State *L)
            {
                lua_pushinteger(L,value);
            }
            
            void set_value(lua_State *L,int index)
            {
                value=lua_tointeger(L,index);
            }
            
            void show()
            {
                Plog::print(value, "\n");
            }
    };
    
    class lua_string: public lua_type
    {
        public:
            std::string value;
            
            lua_string()
                :value("")
            {}
            
            lua_string(std::string const &value_)
                :value(value_)
            {}
            
            void push_value(lua_State *L)
            {
                lua_pushstring(L,value.c_str());
            }
            
            void set_value(lua_State *L,int index)
            {
                value=lua_tostring(L,index);
            }
            
            void show()
            {
                Plog::print(value, "\n");
            }
    };
}

class LuaLoader
{
    private:
        int offset;
        lua_State *L;
    
    public:
        LuaLoader(lua_State *L_,int offset_=1)
            :offset(offset_), L(L_)
        {}
        
        double get_double() { return lua_tonumber(L,offset++); }
        int get_int() { return lua_tointeger(L,offset++); }
        std::string get_string() { return lua_tostring(L,offset++); }

        void reset_offset() { offset=1; }
};

template<class T>
T* create_lua_user_object(lua_State *L,std::string const &meta_name)
{
    T **p=reinterpret_cast<T**>(lua_newuserdata(L,sizeof(T*)));
    *p=new T;
    
    luaL_getmetatable(L,meta_name.c_str());
    lua_setmetatable(L,-2);
    
    return *p;
}

// Binding templates

template<class C,typename... Args,size_t... I>
void call_from_tuple_sub(C *c,void (C::*f)(Args...),std::tuple<Args...> &tuple,std::index_sequence<I...>)
{
    (c->*f)(std::get<I>(tuple)...);
}

template<class C,typename... Args>
void call_from_tuple(C *c,void (C::*f)(Args...),std::tuple<Args...> &tuple)
{
    call_from_tuple_sub(c,f,tuple,std::make_index_sequence<std::tuple_size<std::tuple<Args...>>::value>{});
}

template<typename... Args>
void extract_lua_arguments(lua_State *L,int N,int &val) { val=lua_tointeger(L,N); }

template<typename... Args>
void extract_lua_arguments(lua_State *L,int N,int &val,Args &...args)
{
    val=lua_tointeger(L,N);
    extract_lua_arguments(L,N+1,args...);
}

template<typename... Args>
void extract_lua_arguments(lua_State *L,int N,double &val) { val=lua_tonumber(L,N); }

template<typename... Args>
void extract_lua_arguments(lua_State *L,int N,double &val,Args &...args)
{
    val=lua_tonumber(L,N);
    extract_lua_arguments(L,N+1,args...);
}

template<typename... Args>
void extract_lua_arguments(lua_State *L,int N,std::string &val) { val=lua_tostring(L,N); }

template<typename... Args>
void extract_lua_arguments(lua_State *L,int N,std::string &val,Args &...args)
{
    val=lua_tostring(L,N);
    extract_lua_arguments(L,N+1,args...);
}

template<typename... Args>
void extract_lua_arguments(lua_State *L,int N,std::string const &val) { val=lua_tostring(L,N); }

template<typename... Args>
void extract_lua_arguments(lua_State *L,int N,std::string const &val,Args &...args)
{
    val=lua_tostring(L,N);
    extract_lua_arguments(L,N+1,args...);
}

template<typename... T,size_t... I>
void lua_args_to_tuple_sub(lua_State *L,int N_start,std::tuple<T...> &t,std::index_sequence<I...>)
{
    extract_lua_arguments(L,N_start,std::get<I>(t)...);
}

template<typename... T>
void lua_args_to_tuple(lua_State *L,int N_start,std::tuple<T...> &t)
{
    lua_args_to_tuple_sub(L,N_start,t,std::make_index_sequence<std::tuple_size<std::tuple<T...>>::value>{});
}

template<int ID,class C,typename... Args>
class lua_wrapper
{
    public:
        static void (C::*f)(Args...);
        
        lua_wrapper(void (C::*f_)(Args...))
        {
            f=f_;
        }
        
        static void bind(lua_State *L,std::string const &name,void (C::*f_)(Args...))
        {
            f=f_;
            metatable_add_func(L,name,lua_wrapper<ID,C,Args...>::call);
        }
        
        static int call(lua_State *L)
        {
            C **p_data=reinterpret_cast<C**>(lua_touserdata(L,1));
            C *data=*p_data;
            
            std::tuple<Args...> tuple;
            
            lua_args_to_tuple(L,2,tuple);
            call_from_tuple(data,f,tuple);
            
            return 0;
        }
};

template<int ID,class C,typename... Args>
void (C::*lua_wrapper<ID,C,Args...>::f)(Args...)=nullptr;


template<typename T,typename... Args>
T*& lua_allocate_metapointer(lua_State *L,std::string const &meta_name,Args... args)
{
    T **pp=reinterpret_cast<T**>(lua_newuserdata(L,sizeof(T*)));
    
    luaL_getmetatable(L,meta_name.c_str());
    lua_setmetatable(L,-2);
    
    *pp=new T(args...);
    
    return *pp;
}

template<typename T>
void lua_set_metapointer(lua_State *L,std::string const &meta_name,T *arg)
{
    T **pp=static_cast<T**>(lua_newuserdata(L,sizeof(T*)));
    
    luaL_getmetatable(L,meta_name.c_str());
    lua_setmetatable(L,-2);
    
    *pp=arg;
}

template<typename T>
T* lua_get_metapointer(lua_State *L,int index)
{
    return *(reinterpret_cast<T**>(lua_touserdata(L,index)));
}

//###############
//   base_mode
//###############

class base_mode
{
    public:
        virtual bool interruption_type() { return false; }
        virtual void process() {}
};

#endif // LUA_BASE_H_INCLUDED
