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

#include <lua_structure.h>

#include <sstream>

std::stringstream &  get_stream(lua_State *L)
{
    lua_getglobal(L,"stream");

    std::stringstream *strm=static_cast<std::stringstream*>(lua_touserdata(L,-1));

    return *strm;
}

namespace cnv
{
    int structure_add_block(lua_State *L)
    {
        get_stream(L)
            <<"add_block("<<lua_tostring(L,1)<<","
                          <<lua_tostring(L,2)<<","
                          <<lua_tostring(L,3)<<","
                          <<lua_tostring(L,4)<<","
                          <<lua_tostring(L,5)<<","
                          <<lua_tostring(L,6)<<","
                          <<lua_tointeger(L,7)<<")\n";
        
        return 0;
    }


    int structure_add_conf_coating(lua_State *L)
    {
        get_stream(L)
            <<"add_conformal_coating("<<lua_tostring(L,1)<<","
                                      <<lua_tostring(L,2)<<","
                                      <<lua_tointeger(L,3)<<","
                                      <<lua_tointeger(L,4)<<")\n";
        
        return 0;
    }


    int structure_add_cone(lua_State *L)
    {
        get_stream(L)
            <<"add_cone("<<lua_tostring(L,1)<<","
                         <<lua_tostring(L,2)<<","
                         <<lua_tostring(L,3)<<","
                         <<lua_tostring(L,4)<<","
                         <<lua_tostring(L,5)<<","
                         <<lua_tostring(L,6)<<","
                         <<lua_tostring(L,7)<<","
                         <<lua_tointeger(L,8)<<")\n";
        
        return 0;
    }


    int structure_add_cylinder(lua_State *L)
    {
        get_stream(L)
            <<"add_cylinder("<<lua_tostring(L,1)<<","
                             <<lua_tostring(L,2)<<","
                             <<lua_tostring(L,3)<<","
                             <<lua_tostring(L,4)<<","
                             <<lua_tostring(L,5)<<","
                             <<lua_tostring(L,6)<<","
                             <<lua_tostring(L,7)<<","
                             <<lua_tointeger(L,8)<<")\n";
        
        return 0;
    }


    int structure_add_layer(lua_State *L)
    {
        get_stream(L)
            <<"add_layer(\""<<lua_tostring(L,1)<<"\","
                            <<lua_tostring(L,2)<<","
                            <<lua_tostring(L,3)<<","
                            <<lua_tointeger(L,4)<<")\n";
        
        return 0;
    }


    int structure_add_sphere(lua_State *L)
    {
        get_stream(L)
            <<"add_sphere("<<lua_tostring(L,1)<<","
                           <<lua_tostring(L,2)<<","
                           <<lua_tostring(L,3)<<","
                           <<lua_tostring(L,4)<<","
                           <<lua_tointeger(L,5)<<")\n";

        return 0;
    }


    int structure_declare_parameter(lua_State *L)
    {
        get_stream(L)
            <<"declare_parameter(\""<<lua_tostring(L,1)<<"\","
                                    <<lua_tonumber(L,2)<<")\n";

        return 0;
    }


    int structure_default_material(lua_State *L)
    {
        get_stream(L)
            <<"default_material("<<lua_tointeger(L,1)<<")\n";
        
        return 0;
    }


    int lx(lua_State *L)
    {
        get_stream(L)
            <<"lx="<<lua_tostring(L,1)<<"\n";
        
        return 0;
    }


    int ly(lua_State *L)
    {
        get_stream(L)
            <<"ly="<<lua_tostring(L,1)<<"\n";
        
        return 0;
    }

    int lz(lua_State *L)
    {
        get_stream(L)
            <<"lz="<<lua_tostring(L,1)<<"\n";
        
        return 0;
    }

    int set(lua_State *L)
    {
        get_stream(L)
            <<lua_tostring(L,1)<<"="<<lua_tostring(L,2)<<"\n";
        
        return 0;
    }
}

std::string ageom_to_lua(std::filesystem::path const &script)
{
    std::stringstream strm;

    lua_State *L=luaL_newstate();
    luaL_openlibs(L);

    lua_pushlightuserdata(L,static_cast<void*>(&strm));
    lua_setglobal(L,"stream");

    lua_register(L,"add_block",cnv::structure_add_block);
    //lua_register(L,"add_coating",cnv::structure_add_coating);
    lua_register(L,"add_conformal_coating",cnv::structure_add_conf_coating);
    lua_register(L,"add_cone",cnv::structure_add_cone);
    lua_register(L,"add_cylinder",cnv::structure_add_cylinder);
    //lua_register(L,"add_ellipsoid",cnv::structure_add_ellipsoid);
    //    lua_register(L,"add_height_map",lop_add_height_map);
    lua_register(L,"add_layer",cnv::structure_add_layer);
    //lua_register(L,"add_lua_def",cnv::structure_add_lua_def);
    //lua_register(L,"add_mesh",cnv::structure_add_mesh);
    //lua_register(L,"add_sin_layer",cnv::structure_add_sin_layer);
    lua_register(L,"add_sphere",cnv::structure_add_sphere);
    //lua_register(L,"add_vect_block",cnv::structure_add_vect_block);
    //lua_register(L,"add_vect_tri",cnv::structure_add_vect_tri);
    lua_register(L,"declare_parameter",cnv::structure_declare_parameter);
    lua_register(L,"default_material",cnv::structure_default_material);
    lua_register(L,"lx",cnv::lx);
    lua_register(L,"ly",cnv::ly);
    lua_register(L,"lz",cnv::lz);
    lua_register(L,"set",cnv::set);

    int load_err = luaL_loadfile(L,script.generic_string().c_str());

    if(load_err!=LUA_OK)
    {
        if(load_err==LUA_ERRFILE) Plog::print("Lua file error with ", script, "\n");
        else if(load_err==LUA_ERRSYNTAX) Plog::print("Lua syntax error with ", script, "\n");
        else Plog::print("Lua error with ", script, "\n");
        std::cin.get();
        return "";
    }

    lua_pcall(L, 0, 0, 0);
    
    lua_close(L);

    return strm.str();
}