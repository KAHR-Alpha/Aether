set(gl_src gl_text.cpp
		   gl_utils.cpp
		   gl_vao.cpp
		   gl3w.c)

set(gl_hdr gl_utils.h)

add_library(gl_tools STATIC ${gl_src} ${gl_hdr})

target_include_directories(gl_tools PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(gl_tools PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/gl3w)
target_include_directories(gl_tools PUBLIC ${FREETYPE_INCLUDE_DIRS})

target_link_libraries(gl_tools common_lib)