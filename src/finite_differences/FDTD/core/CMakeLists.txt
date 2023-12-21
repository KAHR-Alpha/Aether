set(fdtd_core_src chpin.cpp
				  fdtd_core.cpp
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
				  
set(fdtd_core_headers em_grid.h
                      fdtd_core.h
                      fdtd_material.h
                      fdtd_utils.h
                      sensors.h
                      sources.h)
			 
add_library(fdtd_core STATIC ${fdtd_core_src} ${fdtd_core_headers})

target_include_directories(fdtd_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(fdtd_core PUBLIC ${CMAKE_SOURCE_DIR}/src/include)
target_link_libraries(fdtd_core common_lib materials)
set_target_properties(fdtd_core PROPERTIES FOLDER "FDTD")