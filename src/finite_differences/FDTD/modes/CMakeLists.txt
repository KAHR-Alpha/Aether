set(fdtd_modes_src default_fdtd.cpp
                   fieldblock_treat.cpp
                   lua_fdtd.cpp
				   normal_incidence.cpp
                   oblique_biphase.cpp
                   oblique_rec.cpp
                   single_particle.cpp)
				  			 
set(fdtd_modes_hdr lua_fdtd.h)

add_library(fdtd_modes STATIC ${fdtd_modes_src} ${fdtd_modes_hdr})
target_include_directories(fdtd_modes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(fdtd_modes fdtd_core fd_core fdtd_sensors fdtd_sources)
set_target_properties(fdtd_modes PROPERTIES FOLDER "FDTD")