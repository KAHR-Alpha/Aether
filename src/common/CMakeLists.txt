set(common_src bitmap.cpp
			   bspline_int.cpp
			   data_hdl.cpp
			   filehdl.cpp
			   foctree.cpp
			   geometry.cpp
			   logger.cpp
			   math_approx.cpp
			   math_bessel.cpp
			   math_optim.cpp
			   math_sym.cpp
			   mathUT.cpp
			   mini_svg.cpp
			   mesh_base.cpp
			   mesh_tools.cpp
			   noise.cpp
			   phys_tools.cpp
			   polygons_2D.cpp
			   ray_intersect.cpp
			   script_utils.cpp
			   spectral_color.cpp
			   string_tools.cpp
			   thread_utils.cpp
			   vector23.cpp
			   voronoi.cpp)

set(common_hdr bitmap3.h
			   bspline_int.h
			   data_hdl.h
			   enum_constants.h
			   enum_tools.h
			   filehdl.h
			   foctree.h
			   geometry.h
			   grid.h
			   logger.h
			   mathUT.h
               math_approx.h
               math_bessel.h
               math_optim.h
               math_sym.h
               mesh_base.h
               mesh_tools.h
			   mini_svg.h
			   noise.h
			   phys_constants.h
			   phys_tools.h
               polygons_2D.h
               ray_intersect.h
			   script_utils.h
			   spectral_color.h
			   string_tools.h
			   thread_utils.h
			   voronoi.h)

add_library(common_lib STATIC ${common_src} ${common_hdr})

target_include_directories(common_lib PUBLIC ${CMAKE_SOURCE_DIR}/src/common)
target_include_directories(common_lib PUBLIC ${EIGEN3_INCLUDE_DIR})
target_include_directories(common_lib PUBLIC ${PNG_PNG_INCLUDE_DIR})

if(WIN32)
	target_link_libraries(common_lib userenv)
endif()

target_link_libraries(common_lib ${PNG_LIBRARY_RELEASE})