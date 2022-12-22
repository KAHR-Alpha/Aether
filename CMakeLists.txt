cmake_minimum_required(VERSION 3.1)

project(Aether CXX C)

message(${CMAKE_GENERATOR})
message(${CMAKE_CXX_COMPILER_ID})

set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 9)
set(CPACK_PACKAGE_VERSION_PATCH 1)

set(TASK "Build CLI" CACHE STRING "What to do")
set_property(CACHE TASK PROPERTY STRINGS "Build CLI" "Build CLI+GUI")

set(INSTALL_PATH "" CACHE PATH "Installation path")
set(NThreads 0 CACHE STRING "Max number of threads" )

# Global setting: Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
set(CMAKE_CXX_FLAGS "-DTHR_NOTIFY_SEPARATE -ftemplate-backtrace-limit=0 -Wno-int-in-bool-context -Wno-attributes" CACHE STRING " " FORCE)
set(CMAKE_INSTALL_PREFIX ${INSTALL_PATH} CACHE STRING "Install path prefix, prepended onto install directories." FORCE)

####################
#   Files Listing
####################

# Sources

set(base_term src/base.cpp)
set(base_gui src/gui_base.cpp)

set(extra_sources src/berreman_strat.cpp
				src/bitmap.cpp
				src/chpin.cpp
				src/data_hdl.cpp
				src/dielec_models.cpp
				src/fd_base.cpp
				src/filehdl.cpp
				src/index_utils.cpp
				src/lua_base.cpp
				src/lua_interface.cpp
				src/lua_material.cpp
				src/material.cpp
				src/mie.cpp
				src/mini_svg.cpp
				src/optical_fibers.cpp
				src/planar_wgd.cpp
				src/script_utils.cpp
				src/thread_utils.cpp)
			
file(GLOB_RECURSE fd_solver src/fd_solver/*.cpp)
file(GLOB_RECURSE fdfd_core src/fdfd/*.cpp)
file(GLOB_RECURSE fdtd_core src/fdtd_core/*.cpp)
# file(GLOB_RECURSE fdtd_modes src/fdtd_modes/*.cpp)
file(GLOB_RECURSE gl_tools_src src/GL_tools/*.cpp src/GL_tools/*.c)
file(GLOB_RECURSE gl3w src/GL_tools/gl3w.c)
file(GLOB_RECURSE gui_src src/GUI/*.cpp)
file(GLOB_RECURSE lua_src src/lua_interface/*.cpp)
file(GLOB_RECURSE math_src src/math_tools/*.cpp)
file(GLOB_RECURSE multilayers_src src/multilayers/*.cpp)
# file(GLOB_RECURSE rcwa_src src/rcwa/*.cpp)
file(GLOB_RECURSE sensor_src src/sensors/*.cpp)
file(GLOB_RECURSE selene_src src/Selene/*.cpp)
file(GLOB_RECURSE source_src src/sources/*.cpp)
file(GLOB_RECURSE structure_src src/structure/*.cpp)

set(fdtd_modes src/fdtd_modes/default_fdtd.cpp
			   src/fdtd_modes/normal_incidence.cpp
			   src/fdtd_modes/oblique_biphase.cpp
			   src/fdtd_modes/oblique_rec.cpp
			   src/fdtd_modes/single_particle.cpp)

file(WRITE REVISION ${CPACK_PACKAGE_VERSION_MAJOR} "." ${CPACK_PACKAGE_VERSION_MINOR} "." ${CPACK_PACKAGE_VERSION_PATCH} )

set(GUI 0)

if(${TASK} STREQUAL "Build CLI+GUI")
	set(GUI 1)
endif()

if(WIN32)
	find_path(EIGEN3_INCLUDE_DIR eigen3.pc.in /usr/lib /usr/local/include libs libs/eigen)
else()
	find_package(Eigen3 REQUIRED)
endif()
find_path(FFTW_INCLUDES fftw3.h /usr/lib /usr/local/include libs libs/fftw64)
find_library(FFTW_LIB NAMES fftw3-3 fftw3 PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib libs libs/fftw64)

find_package(Lua)
find_package(PNG)
find_package(ZLIB)
	
if(NOT WIN32)
	find_package(Threads)
endif()

if(GUI)
	if(WIN32)
		find_path(FREETYPE_INCLUDE_DIRS ft2build.h libs libs/freetype)
		find_library(FREETYPE_LIBRARIES NAMES libfreetype PATHS libs libs/freetpye)
		
		find_path(WXWIDGETS_INCLUDES build.cfg /usr/lib /usr/local/include libs libs/wxWidgets30_h)
		find_library(WXWIDGETS_BASE NAMES wxbase30u_gcc_custom PATHS /usr/lib /usr/local/lib libs libs/wxWidgets30_lib)
		find_library(WXWIDGETS_CORE NAMES wxmsw30u_core_gcc_custom PATHS /usr/lib /usr/local/lib libs libs/wxWidgets30_lib)
		find_library(WXWIDGETS_GL NAMES wxmsw30u_gl_gcc_custom PATHS /usr/lib /usr/local/lib libs libs/wxWidgets30_lib)
		find_library(WXWIDGETS_ADV NAMES wxmsw30u_gl_gcc_custom PATHS /usr/lib /usr/local/lib libs libs/wxWidgets30_lib)
	else()
		find_package(wxWidgets REQUIRED gl core base)
		find_package(Freetype)
	endif()
endif()
				
# add_executable(Aether_CLI ${base_term} ${extra_sources} ${fd_solver}
					  # ${fdfd_core} ${fdtd_core} ${fdtd_modes} ${lua_src}
					  # ${math_src} ${multilayers_src} ${phototracer_src} ${rcwa_src}
					  # ${sensor_src} ${selene_src} ${source_src})

				
add_executable(Aether_CLI ${base_term} ${extra_sources} ${fd_solver}
					  ${fdfd_core} ${fdtd_core} ${fdtd_modes} ${lua_src}
					  ${math_src} ${multilayers_src} ${phototracer_src}
					  ${sensor_src} ${selene_src} ${source_src} ${structure_src})

if(GUI)
	# add_executable(Aether_GUI WIN32 ${base_gui} ${gui_src} ${gl_tools_src}  ${gl3w}
						  # ${extra_sources} ${fd_solver} ${fdfd_core} ${fdtd_core}
						  # ${fdtd_modes} ${lua_src}
						  # ${math_src} ${multilayers_src} ${rcwa_src} ${sensor_src}
						  # ${selene_src} ${source_src})
						  
	add_executable(Aether_GUI WIN32 ${base_gui} ${gui_src} ${gl_tools_src}  ${gl3w}
						  ${extra_sources} ${fd_solver} ${fdfd_core} ${fdtd_core}
						  ${fdtd_modes} ${lua_src}
						  ${math_src} ${multilayers_src} ${sensor_src}
						  ${selene_src} ${source_src} ${structure_src})
						  
	target_compile_definitions(Aether_GUI PUBLIC -DUNICODE)
	target_compile_definitions(Aether_GUI PUBLIC -D_UNICODE)
	
	target_compile_definitions(Aether_GUI PUBLIC -DGUI_ON)
	
	if(NOT WIN32)
		target_compile_definitions(Aether_GUI PUBLIC -DGTK_RMV_RESIZE -DWX30_RESTRICT)
	endif()
endif()

if(OLD_ITERATOR)
	add_definitions(-DLINUX_ITERATOR_TWEAK)
endif()

if(NThreads GREATER 0)
	set_property(SOURCE src/thread_utils.cpp PROPERTY COMPILE_DEFINITIONS MAX_NTHR=${NThreads})
endif()


###############
#   Includes
###############


include_directories(src/include)
include_directories(src/include/fdtd)
include_directories(src/include/gl3w)
include_directories(src/include/GUI)
include_directories(src/include/math_tools)
include_directories(src/include/selene)
include_directories(${LUA_INCLUDE_DIR})
include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${FFTW_INCLUDES})
include_directories(${PNG_PNG_INCLUDE_DIR})

if(GUI)
	target_include_directories(Aether_GUI PUBLIC ${FREETYPE_INCLUDE_DIRS})
	
	if(WIN32)
		target_include_directories(Aether_GUI PUBLIC ${WXWIDGETS_INCLUDES})
	else()
		include(${wxWidgets_USE_FILE})
	endif()
endif()


##############
#   Linking
##############


target_link_libraries(Aether_CLI ${LUA_LIBRARIES})
target_link_libraries(Aether_CLI ${FFTW_LIB})
target_link_libraries(Aether_CLI ${PNG_LIBRARY_RELEASE})
target_link_libraries(Aether_CLI ${ZLIB_LIBRARY_RELEASE})

if(WIN32)
	target_link_libraries(Aether_CLI userenv)
endif()

if(UNIX) 
	target_link_libraries(Aether_CLI dl pthread)
endif()

if(GUI)
	target_link_libraries(Aether_GUI ${LUA_LIBRARIES})
	target_link_libraries(Aether_GUI ${FFTW_LIB})
	target_link_libraries(Aether_GUI ${PNG_LIBRARY_RELEASE})
	target_link_libraries(Aether_GUI ${ZLIB_LIBRARY_RELEASE})
	if(UNIX) 
		target_link_libraries(Aether_CLI dl pthread)
	endif()
	target_link_libraries(Aether_GUI ${FREETYPE_LIBRARIES})
		
	if(WIN32)
		target_link_libraries(Aether_GUI ${WXWIDGETS_BASE})
		target_link_libraries(Aether_GUI ${WXWIDGETS_CORE})
		target_link_libraries(Aether_GUI ${WXWIDGETS_GL})
		target_link_libraries(Aether_GUI ${WXWIDGETS_ADV})
		
		target_link_libraries(Aether_GUI opengl32)
		target_link_libraries(Aether_GUI glu32)
		target_link_libraries(Aether_GUI gdi32)
		target_link_libraries(Aether_GUI ole32)
		target_link_libraries(Aether_GUI oleaut32)
		target_link_libraries(Aether_GUI comctl32)
		target_link_libraries(Aether_GUI comdlg32)
		target_link_libraries(Aether_GUI userenv)
	else()
		target_link_libraries(Aether_GUI ${wxWidgets_LIBRARIES} ${CMAKE_DL_LIBS})
	endif()
endif()


###################
#   Installation
###################

set(BIN_DIR .)
set(RSC_DIR .)

if(NOT WIN32)
	set(BIN_DIR "bin")
	set(RSC_DIR "share/Aether")
endif()

install(TARGETS Aether_CLI RUNTIME DESTINATION ${BIN_DIR} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
install(DIRECTORY contribs DESTINATION ${RSC_DIR})
install(DIRECTORY mat_lib DESTINATION ${RSC_DIR})

# install(DIRECTORY DESTINATION ${RSC_DIR}/buf)
# install(DIRECTORY DESTINATION ${RSC_DIR}/grid)
# install(DIRECTORY DESTINATION ${RSC_DIR}/grid2)
# install(DIRECTORY DESTINATION ${RSC_DIR}/render)

install(DIRECTORY samples DESTINATION ${RSC_DIR})

install(FILES contributors DESTINATION ${RSC_DIR})
install(FILES CHANGELOG DESTINATION ${RSC_DIR})
install(FILES LICENSE DESTINATION ${RSC_DIR})
install(FILES REVISION DESTINATION ${RSC_DIR})

if(WIN32)
	install(FILES ${LUA_LIBRARIES} DESTINATION .)
	install(FILES ${FFTW_LIB} DESTINATION .)
	install(FILES ${PNG_LIBRARY_RELEASE} DESTINATION .)
	install(FILES ${ZLIB_LIBRARY_RELEASE} DESTINATION .)
	
	if(${CMAKE_GENERATOR} STREQUAL "MinGW Makefiles")
		find_file(LIBSTDCPP_DLL libstdc++-6.dll)
		find_file(LIBGCCSEH_DLL libgcc_s_seh-1.dll)
		find_file(LIBWINPTH_DLL libwinpthread-1.dll)
		
		install(FILES ${LIBSTDCPP_DLL} DESTINATION .)
		install(FILES ${LIBGCCSEH_DLL} DESTINATION .)
		install(FILES ${LIBWINPTH_DLL} DESTINATION .)
	endif()
endif()

if(GUI)
	install(TARGETS Aether_GUI RUNTIME DESTINATION ${BIN_DIR} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
	install(DIRECTORY resources DESTINATION ${RSC_DIR})
	
	if(WIN32)
		install(FILES ${WXWIDGETS_BASE} DESTINATION .)
		install(FILES ${WXWIDGETS_CORE} DESTINATION .)
		install(FILES ${WXWIDGETS_GL} DESTINATION .)
		install(FILES ${WXWIDGETS_ADV} DESTINATION .)
	endif()
endif()


set(CPACK_PACKAGE_NAME Aether)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Aether Simulation Suite")
set(CPACK_PACKAGE_VENDOR "Aether Project")
set(CPACK_PACKAGE_CONTACT "aether_project@utt.fr")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Aether")
set(CPACK_PACKAGE_EXECUTABLES Aether_GUI;Aether_GUI)
set(CPACK_PACKAGE_HOMEPAGE_URL https://aether.utt.fr/)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
set(CPACK_VERBATIM_VARIABLES TRUE)

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libfreetype6,libfftw3-bin,zlib1g,libpng16-16,liblua5.4-0")

set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_INSTALL_ROOT "c:")

include(CPack)
