cmake_minimum_required(VERSION 3.22)

project(Aether CXX C)

message(${CMAKE_GENERATOR})
message(${CMAKE_CXX_COMPILER_ID})

function(Dll_Install library_name)
	message("Marking " ${library_name} " for install")
	cmake_path(GET library_name EXTENSION ext)
	if(ext STREQUAL ".dll")
		install(FILES ${library_name} DESTINATION .)
	endif()
	message(Done)
endfunction()

set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 11)
set(CPACK_PACKAGE_VERSION_PATCH 1)

set(TASK "Build CLI" CACHE STRING "What to do")
set_property(CACHE TASK PROPERTY STRINGS "Build CLI" "Build CLI+GUI")

set(GUI 0)

if(TASK STREQUAL "Build CLI+GUI")
	set(GUI 1)
endif()

set(INSTALL_PATH "" CACHE PATH "Installation path")
set(NThreads 0 CACHE STRING "Max number of threads" )

# Global setting: Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)

include(cmake_scripts/find_dependencies.cmake)

if(WIN32 AND NOT MINGW)
	set(CMAKE_CXX_FLAGS "/DTHR_NOTIFY_SEPARATE /DNOMINMAX /EHsc /MP" CACHE STRING " " FORCE)
elseif(WIN32)
	set(CMAKE_CXX_FLAGS "-DTHR_NOTIFY_SEPARATE -ftemplate-backtrace-limit=0 -Wno-int-in-bool-context -Wno-attributes" CACHE STRING " " FORCE)
else()
		set(CMAKE_CXX_FLAGS "-DTHR_NOTIFY_SEPARATE -ftemplate-backtrace-limit=0 -Wno-int-in-bool-context -Wno-attributes" CACHE STRING " " FORCE)
endif()

set(CMAKE_INSTALL_PREFIX ${INSTALL_PATH} CACHE STRING "Install path prefix, prepended onto install directories." FORCE)

####################
#   Files Listing
####################

# Sources
			
file(GLOB_RECURSE fd_solver src/fd_solver/*.cpp)

file(GLOB_RECURSE headers src/include/*.h)
file(GLOB_RECURSE shaders resources/glsl/*.glsl)
file(GLOB_RECURSE tests tests/*.lua)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src/include PREFIX includes FILES ${headers})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/resources/glsl PREFIX shaders FILES ${shaders})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests PREFIX tests FILES ${tests})

file(WRITE REVISION ${CPACK_PACKAGE_VERSION_MAJOR} "." ${CPACK_PACKAGE_VERSION_MINOR} "." ${CPACK_PACKAGE_VERSION_PATCH} )
				
# add_executable(Aether_CLI ${base_term} ${extra_sources} ${fd_solver}
					  # ${headers} ${shaders} ${tests})
					  
add_subdirectory(src/common)
add_subdirectory(src/common_optics)
add_subdirectory(src/finite_differences)
add_subdirectory(src/lua)
add_subdirectory(src/Materials)
add_subdirectory(src/multilayers)
add_subdirectory(src/Selene)
add_subdirectory(src/structure)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

##################
#   Aether Core
##################

add_library(Aether_core STATIC src/core.cpp)

set(core_libraries common_lib
				   common_optics
				   fd_core
				   fdfd
				   fdtd_core
				   fdtd_modes
				   fdtd_sensors
				   fdtd_sources
				   lua_interface
				   materials
				   multilayers
				   structures)

add_dependencies(Aether_core ${core_libraries})
target_link_libraries(Aether_core ${core_libraries})

set_target_properties(Aether_core PROPERTIES FOLDER "Aether")

##########
#   CLI
##########

add_executable(Aether_CLI src/base.cpp)

add_dependencies(Aether_CLI Aether_core)
target_link_libraries(Aether_CLI Aether_core)

set_target_properties(Aether_CLI PROPERTIES FOLDER "Aether")

if(GUI)
	add_subdirectory(src/GL_tools)
	add_subdirectory(src/GUI)
	
	add_executable(Aether_GUI WIN32 src/gui_base.cpp ${shaders} ${tests})
						  
	target_link_libraries(Aether_GUI Aether_core gui_applets gui_fdtd gui_fitters gui_materials gui_multilayers gui_selene gui_teaching)

	target_compile_definitions(Aether_GUI PUBLIC -DUNICODE)
	target_compile_definitions(Aether_GUI PUBLIC -D_UNICODE)
	
	target_compile_definitions(Aether_GUI PUBLIC -DGUI_ON)
	
	if(wxWidgets_FOUND)
		message("wxWidgets definitions: " ${wxWidgets_DEFINITIONS})
		target_compile_definitions(Aether_GUI PUBLIC ${wxWidgets_DEFINITIONS})
	endif()
	
	if(NOT WIN32)
		target_compile_definitions(Aether_GUI PUBLIC -DGTK_RMV_RESIZE -DWX30_RESTRICT)
	endif()
	
	set_target_properties(Aether_GUI PROPERTIES FOLDER "Aether")
endif()
	
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX src FILES ${base_gui} ${gui_src} ${gl_tools_src}  ${gl3w}
																   ${extra_sources} ${fd_solver})
																	   
if(OLD_ITERATOR)
	add_definitions(-DLINUX_ITERATOR_TWEAK)
endif()

if(NThreads GREATER 0)
	set_property(SOURCE src/thread_utils.cpp PROPERTY COMPILE_DEFINITIONS MAX_NTHR=${NThreads})
endif()


###############
#   Includes
###############

message("Lua: " ${LUA_INCLUDE_DIR})
message("Eigen: " ${EIGEN3_INCLUDE_DIR})
message("FFTW: " ${FFTW_INCLUDES})
message("Libpng: " ${PNG_PNG_INCLUDE_DIR})

include_directories(src/include)
include_directories(src/include/fdtd)
include_directories(src/include/gl3w)
include_directories(src/include/GUI)
include_directories(src/include/selene)
include_directories(${LUA_INCLUDE_DIR})
# include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${FFTW_INCLUDES})
# include_directories(${PNG_PNG_INCLUDE_DIR})

if(GUI)
	target_include_directories(Aether_GUI PUBLIC ${FREETYPE_INCLUDE_DIRS})
	
	if(WIN32)
		target_include_directories(Aether_GUI PUBLIC ${WXWIDGETS_INCLUDES})
	else()
		message("wx use file: " ${wxWidgets_USE_FILE})
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
	target_link_libraries(Aether_GUI ${FREETYPE_LIBRARIES})
		
	if(WIN32)
		target_link_libraries(Aether_GUI ${WXWIDGETS_BASE})
		target_link_libraries(Aether_GUI ${WXWIDGETS_CORE})
		target_link_libraries(Aether_GUI ${WXWIDGETS_GL})
		target_link_libraries(Aether_GUI ${WXWIDGETS_ADV})
	else()
		target_link_libraries(Aether_GUI ${wxWidgets_LIBRARIES})
	endif()
	
	if(WIN32)
		target_link_libraries(Aether_GUI opengl32)
		target_link_libraries(Aether_GUI glu32)
		target_link_libraries(Aether_GUI gdi32)
		target_link_libraries(Aether_GUI ole32)
		target_link_libraries(Aether_GUI oleaut32)
		target_link_libraries(Aether_GUI comctl32)
		target_link_libraries(Aether_GUI comdlg32)
		target_link_libraries(Aether_GUI userenv)
		
		if(MSVC)
			target_link_libraries(Aether_GUI ${WXWIDGETS_PNG})
			target_link_libraries(Aether_GUI ${WXWIDGETS_JPEG})
			target_link_libraries(Aether_GUI ${WXWIDGETS_TIFF})
			target_link_libraries(Aether_GUI ${WXWIDGETS_ZLIB})
			target_link_libraries(Aether_GUI rpcrt4)
		endif()
	else()
		target_link_libraries(Aether_GUI ${CMAKE_DL_LIBS})
	endif()
endif()

##########################
#   IDE setup and tests
##########################

include(cmake_scripts/IDE_setup.cmake)
include(cmake_scripts/unit_tests.cmake)

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

install(DIRECTORY samples DESTINATION ${RSC_DIR})

install(FILES contributors DESTINATION ${RSC_DIR})
install(FILES CHANGELOG DESTINATION ${RSC_DIR})
install(FILES LICENSE DESTINATION ${RSC_DIR})
install(FILES REVISION DESTINATION ${RSC_DIR})

if(WIN32)
	Dll_Install(${LUA_LIBRARIES})
	Dll_Install(${FFTW_LIB})
	Dll_Install(${PNG_LIBRARY_RELEASE})
	Dll_Install(${ZLIB_LIBRARY_RELEASE})
	
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
		Dll_Install(${WXWIDGETS_BASE})
		Dll_Install(${WXWIDGETS_CORE})
		Dll_Install(${WXWIDGETS_GL})
		Dll_Install(${WXWIDGETS_ADV})
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
