set(fdtd_modes_src default_fdtd.cpp
                   normal_incidence.cpp
                   oblique_biphase.cpp
                   oblique_rec.cpp
                   single_particle.cpp)
				  
set(fdtd_modes_headers ${CMAKE_SOURCE_DIR}/src/include/lua_fd.h)
			 
add_library(fdtd_modes STATIC ${fdtd_modes_src} ${fdtd_modes_headers})

target_include_directories(fdtd_modes PUBLIC ${CMAKE_SOURCE_DIR}/src/include/fdtd)
target_include_directories(fdtd_modes PUBLIC ${CMAKE_SOURCE_DIR}/src/include/)

add_dependencies(fdtd_modes fdtd_core lua_interface)
target_link_libraries(fdtd_modes fdtd_core lua_interface)

set_target_properties(fdtd_modes PROPERTIES FOLDER "FDTD")