set(lua_core_src lua_base.cpp)
			 
set(lua_core_headers lua_base.h)

add_library(lua_core STATIC ${lua_core_src} ${lua_core_headers})
target_include_directories(lua_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(lua_core PUBLIC ${LUA_INCLUDE_DIR})
target_link_libraries(lua_core common_lib)