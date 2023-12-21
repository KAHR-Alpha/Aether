set(sensors_src fieldblock_holder.cpp
                lua_sensors.cpp
                sensors.cpp
                s_completion.cpp
                s_diffrac.cpp
                s_farfield.cpp
                s_fieldmap.cpp
                s_modes.cpp
                s_movie.cpp
                s_power.cpp
                s_spec_power.cpp)
			 
set(sensors_headers ../core/sensors.h fieldblock_holder.h lua_sensors.h)

add_library(fdtd_sensors STATIC ${sensors_src} ${sensors_headers})
target_include_directories(fdtd_sensors PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(fdtd_sensors PUBLIC ${FFTW_INCLUDES})
target_link_libraries(fdtd_sensors fdtd_core lua_core multilayers)

set_target_properties(fdtd_sensors PROPERTIES FOLDER "FDTD")