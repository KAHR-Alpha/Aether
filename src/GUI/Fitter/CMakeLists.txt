set(gui_fitter_src gui_fitter.cpp
				   gui_fitter_functions.cpp
				   ../gui_layerfitter.cpp
				   ../gui_matsfitter.cpp)

set(gui_fitter_hdr ../../include/GUI/gui_fitter.h
				   ../../include/GUI/gui_fitter_functions.h
				   ../../include/GUI/gui_layerfitter.h
				   ../../include/GUI/gui_matsfitter.h)

add_library(gui_fitters STATIC ${gui_fitter_src} ${gui_fitter_hdr})

target_include_directories(gui_fitters PUBLIC ../../include/GUI)

target_link_libraries(gui_fitters gui_common)