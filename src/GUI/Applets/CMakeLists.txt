set(applet_src gui_curve_extractor.cpp
			   gui_diffract_orders.cpp
			   gui_diffract_pattern.cpp
			   gui_fd_1ms.cpp
			   gui_fd_1ms_ha.cpp
			   gui_fd_1ms_ha_gl.cpp
			   gui_geometric_gradient.cpp
			   gui_mie.cpp
			   gui_optical_fibers.cpp
			   gui_parametric_interp.cpp
			   gui_parametric_visualizer.cpp
			   gui_samples_explorer.cpp
			   gui_spp.cpp)
			   
set(applet_hdr gui_curve_extractor.h
               gui_diffract_orders.h
               gui_diffract_pattern.h
               gui_fd_ms.h
               gui_geometric_gradient.h
               gui_mie.h
               gui_optical_fibers.h
               gui_parametric_interp.h
               gui_parametric_visualizer.h
               gui_samples_explorer.h
               gui_spp.h)

add_library(gui_applets STATIC ${applet_src} ${applet_hdr})
target_include_directories(gui_applets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gui_applets gui_common gui_materials gui_multilayers)

set_target_properties(gui_applets PROPERTIES FOLDER "GUI")