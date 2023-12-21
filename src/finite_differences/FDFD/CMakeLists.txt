set(fdfd_sources fd_solver.cpp
				 fdfd.cpp
                 fdfd_mode.cpp
                 fdfd_sam.cpp
                 fdfd_solvers.cpp
                 lua_fdfd.cpp)
				  
set(fdfd_headers fdfd.h lua_fdfd.h)
			 
add_library(fdfd STATIC ${fdfd_sources} ${fdfd_headers})
target_include_directories(fdfd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(fdfd fd_core)