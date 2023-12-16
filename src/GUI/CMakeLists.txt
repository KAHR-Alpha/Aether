add_subdirectory(FDTD)
add_subdirectory(Fitter)
add_subdirectory(Materials)
add_subdirectory(multilayers)
add_subdirectory(Selene)
add_subdirectory(Teaching)

set(gui_common_src gui_utils.cpp)

set(gui_common_headers ../include/GUI/aether.h
					   ../include/GUI/gui.h)

add_library(gui_common STATIC ${gui_common_src} ${gui_common_headers})
