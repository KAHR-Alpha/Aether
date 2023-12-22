set(mltly_sources berreman_strat.cpp
			      index_utils.cpp
			      multilayers_berreman.cpp
			      multilayers_eh.cpp
			      multilayers_fresnel.cpp
			      multilayers_ud.cpp
				  planar_wgd.cpp)
			 
set(mltly_headers berreman_strat.h
                  index_utils.h
				  multilayers.h
				  planar_wgd.h)

add_library(multilayers STATIC ${mltly_sources} ${mltly_headers})
target_include_directories(multilayers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(multilayers common_lib materials)