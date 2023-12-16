set(sensors_src fieldblock_holder.cpp
                sensors.cpp
                s_completion.cpp
                s_diffrac.cpp
                s_farfield.cpp
                s_fieldmap.cpp
                s_modes.cpp
                s_movie.cpp
                s_power.cpp
                s_spec_power.cpp)
			 
set(sensors_headers ${CMAKE_SOURCE_DIR}/src/include/fdtd/sensors.h)

add_library(fdtd_sensors STATIC ${sensors_src} ${sensors_headers})

target_include_directories(fdtd_sensors PUBLIC ${CMAKE_SOURCE_DIR}/src/include/fdtd)
target_include_directories(fdtd_sensors PUBLIC ${CMAKE_SOURCE_DIR}/src/include/)
target_include_directories(fdtd_sensors PUBLIC ${FFTW_INCLUDES})

add_dependencies(fdtd_sensors fdtd_core lua_interface)
target_link_libraries(fdtd_sensors fdtd_core lua_interface)

set_target_properties(fdtd_sensors PROPERTIES FOLDER "FDTD")