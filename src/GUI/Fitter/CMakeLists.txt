set(gui_fitter_src gui_fitter.cpp
				   gui_fitter_functions.cpp
				   gui_layerfitter.cpp
				   gui_matsfitter.cpp
				   gui_matpanel.cpp)

set(gui_fitter_hdr gui_fitter.h
				   gui_fitter_functions.h
				   gui_layerfitter.h
				   gui_matsfitter.h
				   gui_matspanel.h)

add_library(gui_fitters STATIC ${gui_fitter_src} ${gui_fitter_hdr})
target_include_directories(gui_fitters PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gui_fitters gui_common)

set_target_properties(gui_fitters PROPERTIES FOLDER "GUI")