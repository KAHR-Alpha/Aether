set(fdfd_sources fdfd.cpp
                 fdfd_mode.cpp
                 fdfd_sam.cpp
                 fdfd_solvers.cpp)
				  
set(fdfd_headers ${CMAKE_SOURCE_DIR}/src/include/fdfd.h)
			 
add_library(fdfd STATIC ${fdfd_sources} ${fdfd_headers})

target_include_directories(fdfd PUBLIC ${CMAKE_SOURCE_DIR}/src/include/)

add_dependencies(fdfd maths_library lua_interface)
target_link_libraries(fdfd maths_library lua_interface)