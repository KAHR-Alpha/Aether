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

set(gui_selene_hdr ../../include/GUI/gui_selene.h
				   ../../include/GUI/gui_selene_gl.h
				   ../../include/GUI/gui_selene_sensor.h)

add_library(gui_selene STATIC ${gui_selene_src} ${gui_selene_hdr})

target_include_directories(gui_selene PUBLIC ../../include/GUI)

target_link_libraries(gui_selene gui_common)