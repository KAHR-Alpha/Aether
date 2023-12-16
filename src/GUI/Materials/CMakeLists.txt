set(gui_material_src gui_material_editor.cpp
                     gui_material_editor_panels.cpp
                     gui_material_library.cpp
                     gui_material_lua.cpp
                     gui_material_manager.cpp
                     gui_material_selector.cpp
                     gui_mini_material_selector.cpp)

add_library(gui_materials STATIC ${gui_material_src})

target_link_libraries(gui_materials gui_common)