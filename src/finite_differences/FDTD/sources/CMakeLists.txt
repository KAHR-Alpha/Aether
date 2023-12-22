set(sources_src lua_sources.cpp
                sc_afp_tfsf.cpp
                sc_bloch.cpp
                sc_electric_dipole.cpp
                sc_gaussian_beam.cpp
                sc_guided_planar.cpp
                sc_oscillator.cpp
                sources.cpp)
			 
set(sourcess_headers ../core/sources.h lua_sources.h)

add_library(fdtd_sources STATIC ${sources_src} ${sourcess_headers})
target_include_directories(fdtd_sources PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(fdtd_sources fdtd_modes fdfd multilayers)

set_target_properties(fdtd_sources PROPERTIES FOLDER "Finite Differences")