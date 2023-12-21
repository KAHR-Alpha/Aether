set(fd_core_src fd_base.cpp
				lua_fd.cpp)
				  
set(fd_core_hdr fd_base.h lua_fd.h)
			 
add_library(fd_core STATIC ${fd_core_src} ${fd_core_hdr})
target_include_directories(fd_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(fd_core common_lib materials structures)