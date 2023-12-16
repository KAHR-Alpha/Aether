set(gui_mutilayers_src ../gui_multilayers.cpp
					   ../gui_multilayers_utils.cpp
					   gui_multilayers_lua.cpp)
					   
set(gui_multilayers_hdr ../../include/GUI/gui_multilayers.h
						../../include/GUI/gui_multilayers_utils.h)

add_library(gui_multilayers STATIC ${gui_mutilayers_src} ${gui_multilayers_hdr})

target_include_directories(gui_multilayers PUBLIC "../../include/GUI")

target_link_libraries(gui_multilayers maths_library gui_common)