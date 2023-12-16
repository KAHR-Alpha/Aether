set(math_src bspline_int.cpp
			 foctree.cpp
			 geometry.cpp
			 math_approx.cpp
			 math_bessel.cpp
			 math_optim.cpp
			 math_sym.cpp
			 mathUT.cpp
			 mesh_base.cpp
			 mesh_tools.cpp
			 noise.cpp
			 phys_tools.cpp
			 polygons_2D.cpp
			 ray_intersect.cpp
			 spectral_color.cpp
			 string_tools.cpp
			 vector23.cpp
			 voronoi.cpp)
			 
set(math_headers ${CMAKE_SOURCE_DIR}/src/include/math_tools/bspline_int.h
                 ${CMAKE_SOURCE_DIR}/src/include/math_tools/foctree.h
                 ${CMAKE_SOURCE_DIR}/src/include/math_tools/geometry.h
                 ${CMAKE_SOURCE_DIR}/src/include/math_tools/grid.h
                 ${CMAKE_SOURCE_DIR}/src/include/math_tools/mathUT.h
                 ${CMAKE_SOURCE_DIR}/src/include/math_tools/math_approx.h
                 ${CMAKE_SOURCE_DIR}/src/include/math_tools/math_bessel.h
                 ${CMAKE_SOURCE_DIR}/src/include/math_tools/math_optim.h
                 ${CMAKE_SOURCE_DIR}/src/include/math_tools/math_sym.h
                 ${CMAKE_SOURCE_DIR}/src/include/math_tools/mesh_base.h
                 ${CMAKE_SOURCE_DIR}/src/include/math_tools/mesh_tools.h
                 ${CMAKE_SOURCE_DIR}/src/include/math_tools/noise.h
                 ${CMAKE_SOURCE_DIR}/src/include/math_tools/phys_tools.h
                 ${CMAKE_SOURCE_DIR}/src/include/math_tools/polygons_2D.h
                 ${CMAKE_SOURCE_DIR}/src/include/math_tools/ray_intersect.h
                 ${CMAKE_SOURCE_DIR}/src/include/math_tools/spectral_color.h
                 ${CMAKE_SOURCE_DIR}/src/include/math_tools/string_tools.h
                 ${CMAKE_SOURCE_DIR}/src/include/math_tools/voronoi.h)

add_library(maths_library STATIC ${math_src} ${math_headers})

target_include_directories(maths_library PUBLIC ${CMAKE_SOURCE_DIR}/src/include/math_tools)
target_include_directories(maths_library PUBLIC ${CMAKE_SOURCE_DIR}/src/include)
target_include_directories(maths_library PUBLIC ${EIGEN3_INCLUDE_DIR})
target_include_directories(maths_library PUBLIC ${PNG_PNG_INCLUDE_DIR})