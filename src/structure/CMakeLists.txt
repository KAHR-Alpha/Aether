set(struct_sources ageom_converter.cpp
				   lua_structure.cpp
				   structure.cpp
				   structure_primitives.cpp)
				   
set(struct_headers lua_structure.h structure.h)

add_library(structures STATIC ${struct_sources} ${struct_headers})
target_include_directories(structures PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(structures common_lib lua_core)