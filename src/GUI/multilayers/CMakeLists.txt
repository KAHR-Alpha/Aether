set(gui_mutilayers_src gui_multilayers.cpp
					   gui_multilayers_utils.cpp
					   gui_multilayers_lua.cpp)
					   
set(gui_multilayers_hdr gui_multilayers.h
						gui_multilayers_utils.h)

add_library(gui_multilayers STATIC ${gui_mutilayers_src} ${gui_multilayers_hdr})
target_include_directories(gui_multilayers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gui_multilayers gui_common gui_materials)

set_target_properties(gui_multilayers PROPERTIES FOLDER "GUI")