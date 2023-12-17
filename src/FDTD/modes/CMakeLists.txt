set(fdtd_modes_src default_fdtd.cpp
                   fieldblock_treat.cpp
                   normal_incidence.cpp
                   oblique_biphase.cpp
                   oblique_rec.cpp
                   single_particle.cpp)
				  			 
add_library(fdtd_modes STATIC ${fdtd_modes_src})
target_link_libraries(fdtd_modes fdtd_core lua_interface)
set_target_properties(fdtd_modes PROPERTIES FOLDER "FDTD")