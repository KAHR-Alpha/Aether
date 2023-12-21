set(common_src mie.cpp
			   optical_fibers.cpp)

set(common_hdr mie.h
			   optical_fibers.h)

add_library(common_optics STATIC ${common_src} ${common_hdr})

target_include_directories(common_optics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common_optics common_lib materials lua_interface)