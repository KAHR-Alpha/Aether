set(selene_src sel_analysis.cpp
			   sel_irf.cpp
			   sel_light.cpp
			   sel_mesh.cpp
			   sel_obj.cpp
			   sel_obj_surface.cpp
			   sel_obj_volume.cpp
			   selene.cpp)
			 
set(selene_headers ${CMAKE_SOURCE_DIR}/src/include/selene/selene.h
                   ${CMAKE_SOURCE_DIR}/src/include/selene/selene_mesh.h)

add_library(selene_core STATIC ${selene_src} ${selene_headers})

target_include_directories(selene_core PUBLIC ${CMAKE_SOURCE_DIR}/src/include/selene)
target_include_directories(selene_core PUBLIC ${CMAKE_SOURCE_DIR}/src/include/)

target_link_libraries(selene_core maths_library)