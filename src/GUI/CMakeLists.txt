add_subdirectory(FDTD)
add_subdirectory(Fitter)
add_subdirectory(Materials)
add_subdirectory(multilayers)
add_subdirectory(Selene)
add_subdirectory(Teaching)

set(gui_common_src gui_gl_fd.cpp
				   gui_graph.cpp
				   gui_gl_graph.cpp
				   gui_matpanel.cpp
				   gui_panels_list.cpp
				   gui_rsc.cpp
				   gui_utils.cpp)

set(gui_common_headers ../include/GUI/aether.h
					   ../include/GUI/gui.h
					   ../include/GUI/gui_gl_fd.h
					   ../include/GUI/gui_gl_graph.h
					   ../include/GUI/gui_graph.h
					   ../include/GUI/gui_matspanel.h
					   ../include/GUI/gui_rsc.h
					   ../include/GUI/gui_panels_list.h)

add_library(gui_common STATIC ${gui_common_src} ${gui_common_headers})

target_include_directories(gui_common PUBLIC ../include/GUI)
target_include_directories(gui_common PUBLIC ../include)
target_include_directories(gui_common PUBLIC ${FREETYPE_INCLUDE_DIRS})

if(WIN32)
	target_include_directories(gui_common PUBLIC ${WXWIDGETS_INCLUDES})
else()
	message("wx use file: " ${wxWidgets_USE_FILE})
	include(${wxWidgets_USE_FILE})
endif()

target_link_libraries(gui_common Aether_core gl_tools)

#####################
#   Simple applets
#####################

set(applet_src gui_diffract_orders.cpp
			   gui_fd_1ms_ha.cpp
			   gui_fd_1ms_ha_gl.cpp
			   gui_geometric_gradient.cpp
			   gui_layerfitter.cpp
			   gui_mie.cpp
			   gui_samples_explorer.cpp
			   gui_optical_fibers.cpp
			   gui_parametric_visualizer.cpp
			   gui_spp.cpp)
			   
set(applet_hdr ../include/GUI/gui_diffract_orders.h
               ../include/GUI/gui_geometric_gradient.h
               ../include/GUI/gui_layerfitter.h
               ../include/GUI/gui_mie.h
               ../include/GUI/gui_samples_explorer.h
               ../include/GUI/gui_optical_fibers.h
               ../include/GUI/gui_parametric_visualizer.h
               ../include/GUI/gui_spp.h)

add_library(gui_applets STATIC ${applet_src} ${applet_hdr})

target_link_libraries(gui_applets gui_common)