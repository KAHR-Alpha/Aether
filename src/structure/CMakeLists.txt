set(struct_sources structure.cpp
				   structure_primitives.cpp)
				   
set(struct_headers ${CMAKE_SOURCE_DIR}/src/include/structure.h)

add_library(structures STATIC ${struct_sources} ${struct_headers})

target_include_directories(structures PUBLIC ${CMAKE_SOURCE_DIR}/src/include/)

target_link_libraries(structures common_lib lua_interface)