set(lua_src lua_fd.cpp
			lua_material.cpp
			lua_multilayers.cpp
			lua_optim.cpp
			lua_selene.cpp
			lua_selene_element.cpp
			lua_selene_light.cpp
			lua_selene_target.cpp
			lua_structure.cpp
			../lua_base.cpp
			../lua_interface.cpp)
			 
set(lua_headers ${CMAKE_SOURCE_DIR}/src/include/lua_base.h
                ${CMAKE_SOURCE_DIR}/src/include/lua_fd.h
                ${CMAKE_SOURCE_DIR}/src/include/lua_interface.h
                ${CMAKE_SOURCE_DIR}/src/include/lua_material.h
                ${CMAKE_SOURCE_DIR}/src/include/lua_multilayers.h
                ${CMAKE_SOURCE_DIR}/src/include/lua_selene.h
                ${CMAKE_SOURCE_DIR}/src/include/lua_structure.h)

add_library(lua_interface STATIC ${lua_src} ${lua_headers})

target_include_directories(lua_interface PUBLIC ${CMAKE_SOURCE_DIR}/src/include/)
target_include_directories(lua_interface PUBLIC ${LUA_INCLUDE_DIR})

target_link_libraries(lua_interface common_lib fdtd_core selene_core)