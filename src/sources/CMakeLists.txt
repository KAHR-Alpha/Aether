set(sources_src sc_afp_tfsf.cpp
                sc_bloch.cpp
                sc_electric_dipole.cpp
                sc_gaussian_beam.cpp
                sc_guided_planar.cpp
                sc_oscillator.cpp
                sources.cpp)
			 
set(sourcess_headers ${CMAKE_SOURCE_DIR}/src/include/fdtd/sources.h)

add_library(fdtd_sources STATIC ${sources_src} ${sourcess_headers})

target_include_directories(fdtd_sources PUBLIC ${CMAKE_SOURCE_DIR}/src/include/fdtd)
target_include_directories(fdtd_sources PUBLIC ${CMAKE_SOURCE_DIR}/src/include/)

add_dependencies(fdtd_sources fdtd_core lua_interface)
target_link_libraries(fdtd_sources fdtd_core lua_interface)

set_target_properties(fdtd_sources PROPERTIES FOLDER "FDTD")