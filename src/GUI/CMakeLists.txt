add_subdirectory(Applets)
add_subdirectory(FDTD)
add_subdirectory(Fitter)
add_subdirectory(Materials)
add_subdirectory(multilayers)
add_subdirectory(Selene)
add_subdirectory(Teaching)

set(gui_common_src gui_gl_fd.cpp
				   gui_graph.cpp
				   gui_gl_graph.cpp
				   gui_panels_list.cpp
				   gui_rsc.cpp
				   gui_utils.cpp)

set(gui_common_headers aether.h
					   gui.h
					   gui_gl_fd.h
					   gui_gl_graph.h
					   gui_graph.h
					   gui_rsc.h
					   gui_panels_list.h)

add_library(gui_common STATIC ${gui_common_src} ${gui_common_headers})

target_include_directories(gui_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(gui_common PUBLIC ../include)
target_include_directories(gui_common PUBLIC ${FREETYPE_INCLUDE_DIRS})

target_include_directories(gui_common PUBLIC ${WXWIDGETS_INCLUDES})

target_link_libraries(gui_common Aether_core gl_tools)

set_target_properties(gui_common PROPERTIES FOLDER "GUI")