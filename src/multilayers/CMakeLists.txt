set(mltly_sources multilayers_berreman.cpp
			      multilayers_eh.cpp
			      multilayers_fresnel.cpp
			      multilayers_ud.cpp)
			 
set(mltly_headers ${CMAKE_SOURCE_DIR}/src/include/berreman_strat.h
                  ${CMAKE_SOURCE_DIR}/src/include/multilayers.h)

add_library(multilayers STATIC ${mltly_sources} ${mltly_headers})

target_include_directories(multilayers PUBLIC ${CMAKE_SOURCE_DIR}/src/include/)

target_link_libraries(multilayers common_lib)