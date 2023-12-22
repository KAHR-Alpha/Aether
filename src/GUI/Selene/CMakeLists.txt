set(gui_selene_src gui_selene.cpp
                   gui_selene_element.cpp
                   gui_selene_gl.cpp
                   gui_selene_lua.cpp
                   gui_selene_materials.cpp
                   gui_selene_object.cpp
                   gui_selene_object_mesh.cpp
                   gui_selene_optimization.cpp
                   gui_selene_sensor.cpp
                   gui_selene_source.cpp)

set(gui_selene_hdr gui_selene.h
				   gui_selene_gl.h
				   gui_selene_sensor.h)

add_library(gui_selene STATIC ${gui_selene_src} ${gui_selene_hdr})
target_include_directories(gui_selene PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gui_selene gui_common gui_materials gui_multilayers)

set_target_properties(gui_selene PROPERTIES FOLDER "GUI")