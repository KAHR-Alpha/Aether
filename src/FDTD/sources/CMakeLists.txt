set(sources_src sc_afp_tfsf.cpp
                sc_bloch.cpp
                sc_electric_dipole.cpp
                sc_gaussian_beam.cpp
                sc_guided_planar.cpp
                sc_oscillator.cpp
                sources.cpp)
			 
set(sourcess_headers ../core/sources.h)

add_library(fdtd_sources STATIC ${sources_src} ${sourcess_headers})
target_link_libraries(fdtd_sources fdtd_core lua_interface)
set_target_properties(fdtd_sources PROPERTIES FOLDER "FDTD")