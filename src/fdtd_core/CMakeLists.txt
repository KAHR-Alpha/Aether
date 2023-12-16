set(fdtd_core_src fdtd_core.cpp
                  fdtd_core_aniso.cpp
                  fdtd_pml.cpp
                  fdtd_threads.cpp
                  fdtd_utils.cpp
                  mats.cpp
                  mats_aniD_const.cpp
                  mats_ani_const.cpp
                  mats_const.cpp
                  mats_levels.cpp
                  mats_PCRC.cpp
                  mats_RC.cpp)
				  
set(fdtd_core_headers ${CMAKE_SOURCE_DIR}/src/include/fdtd/em_grid.h
                      ${CMAKE_SOURCE_DIR}/src/include/fdtd/fdtd_core.h
                      ${CMAKE_SOURCE_DIR}/src/include/fdtd/fdtd_material.h
                      ${CMAKE_SOURCE_DIR}/src/include/fdtd/fdtd_utils.h
                      ${CMAKE_SOURCE_DIR}/src/include/fdtd/sensors.h
                      ${CMAKE_SOURCE_DIR}/src/include/fdtd/sources.h)
			 
add_library(fdtd_core STATIC ${fdtd_core_src} ${fdtd_core_headers})

target_include_directories(fdtd_core PUBLIC ${CMAKE_SOURCE_DIR}/src/include/fdtd)
target_include_directories(fdtd_core PUBLIC ${CMAKE_SOURCE_DIR}/src/include/)

target_link_libraries(fdtd_core maths_library)

set_target_properties(fdtd_core PROPERTIES FOLDER "FDTD")