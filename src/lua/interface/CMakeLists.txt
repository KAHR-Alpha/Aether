set(lua_src lua_interface.cpp
			lua_multilayers.cpp
			lua_optim.cpp
			lua_selene.cpp
			lua_selene_analysis.cpp
			lua_selene_element.cpp
			lua_selene_light.cpp
			lua_selene_target.cpp)
			 
set(lua_headers lua_interface.h
                lua_multilayers.h
                lua_selene.h)

add_library(lua_interface STATIC ${lua_src} ${lua_headers})
target_include_directories(lua_interface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lua_interface lua_core fdtd_core selene_core fdtd_sensors materials multilayers structures)