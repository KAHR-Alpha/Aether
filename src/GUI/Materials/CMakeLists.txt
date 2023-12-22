set(gui_material_src gui_material_editor.cpp
                     gui_material_editor_panels.cpp
                     gui_material_library.cpp
                     gui_material_lua.cpp
                     gui_material_manager.cpp
                     gui_material_selector.cpp
                     gui_mini_material_selector.cpp)

set(gui_material_hdr gui_material.h
					 gui_material_editor.h
					 gui_material_editor_panels.h
					 gui_material_library.h)
					 

add_library(gui_materials STATIC ${gui_material_src} ${gui_material_hdr})
target_include_directories(gui_materials PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gui_materials gui_common)

set_target_properties(gui_materials PROPERTIES FOLDER "GUI")