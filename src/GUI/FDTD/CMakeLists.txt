set(gui_fdtd_src gui_fdtd.cpp
                 gui_fdtd_gl.cpp
                 gui_fdtd_misc_dialogs.cpp
                 gui_fdtd_mode_dialog.cpp
                 gui_fdtd_structdesign.cpp
                 gui_fdtd_structdesign_panels.cpp
                 gui_fdtd_structdesign_variables.cpp
                 gui_fieldblock_explr.cpp)

set(gui_fdtd_hdr gui_fdtd.h
				 gui_fdtd_structdesign.h
				 gui_fdtd_structdesign_variables.h
				 gui_fieldblock_explr.h)

add_library(gui_fdtd STATIC ${gui_fdtd_src} ${gui_fdtd_hdr})
target_link_libraries(gui_fdtd gui_common gui_materials fdtd_modes fdtd_sources fdtd_sensors)
target_include_directories(gui_fdtd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(gui_fdtd PROPERTIES FOLDER "GUI")