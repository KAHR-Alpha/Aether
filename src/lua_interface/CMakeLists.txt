set(lua_src lua_base.cpp
			lua_fd.cpp
			lua_interface.cpp
			lua_material.cpp
			lua_multilayers.cpp
			lua_optim.cpp
			lua_selene.cpp
			lua_selene_element.cpp
			lua_selene_light.cpp
			lua_selene_target.cpp
			lua_structure.cpp)
			 
set(lua_headers lua_base.h
                lua_fd.h
                lua_interface.h
                lua_material.h
                lua_multilayers.h
                lua_selene.h
                lua_structure.h)

add_library(lua_interface STATIC ${lua_src} ${lua_headers})
target_include_directories(lua_interface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(lua_interface PUBLIC ${LUA_INCLUDE_DIR})
target_link_libraries(lua_interface common_lib fdtd_core selene_core)