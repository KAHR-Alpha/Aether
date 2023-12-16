set(gui_teaching_src gui_ellifr.cpp
					 gui_ellipso3.cpp
					 gui_sem.cpp)

set(gui_teaching_hdr ../../include/GUI/gui_ellifr.h
					 ../../include/GUI/gui_ellipso3.h
					 ../../include/GUI/gui_sem.h)

add_library(gui_teaching ${gui_teaching_src} ${gui_teaching_hdr})

target_link_libraries(gui_teaching gui_common)