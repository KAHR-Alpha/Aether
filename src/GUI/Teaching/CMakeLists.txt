set(gui_teaching_src gui_ellifr.cpp
					 gui_ellipso3.cpp
					 gui_sem.cpp)

set(gui_teaching_hdr gui_ellifr.h
					 gui_ellipso3.h
					 gui_sem.h)

add_library(gui_teaching ${gui_teaching_src} ${gui_teaching_hdr})
target_include_directories(gui_teaching PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gui_teaching gui_common gui_materials)

set_target_properties(gui_teaching PROPERTIES FOLDER "GUI")