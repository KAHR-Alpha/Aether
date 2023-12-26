
############
#   Eigen
############

set(EIGEN3_INCLUDE_DIR CACHE PATH "")

if(EIGEN3_INCLUDE_DIR STREQUAL "")
	unset(EIGEN3_INCLUDE_DIR CACHE)
	find_package(Eigen3 REQUIRED)
endif()


###########
#   FFTW
###########

set(FFTW_INCLUDES CACHE PATH "")
set(FFTW_LIB CACHE PATH "")

if(FFTW_INCLUDES STREQUAL "" OR FFTW_LIB STREQUAL "")
	unset(FFTW_INCLUDES CACHE)
	unset(FFTW_LIB CACHE)
	find_path(FFTW_INCLUDES fftw3.h /usr/lib /usr/local/include libs libs/fftw64)
	find_library(FFTW_LIB NAMES fftw3-3 fftw3 PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib libs libs/fftw64)
	# find_package(FFTW3)
endif()


##########
#   Lua
##########

set(LUA_INCLUDE_DIR CACHE PATH "")
set(LUA_LIBRARIES CACHE PATH "")

if(LUA_INCLUDE_DIR STREQUAL "" OR LUA_LIBRARIES STREQUAL "")
	unset(LUA_INCLUDE_DIR CACHE)
	unset(LUA_LIBRARIES CACHE)
	find_package(Lua)
endif()


#############
#   Libpng
#############

set(PNG_PNG_INCLUDE_DIR CACHE PATH "")
set(PNG_LIBRARY_RELEASE CACHE PATH "")

if(PNG_PNG_INCLUDE_DIR STREQUAL "" OR PNG_LIBRARY_RELEASE STREQUAL "")
	unset(PNG_PNG_INCLUDE_DIR CACHE)
	unset(PNG_LIBRARY_RELEASE CACHE)
	find_package(PNG)
endif()


###########
#   zlib
###########

set(ZLIB_LIBRARY_RELEASE CACHE PATH "")

if(ZLIB_LIBRARY_RELEASE STREQUAL "")
	unset(ZLIB_LIBRARY_RELEASE CACHE)
	find_package(ZLIB)
endif()
	
if(NOT WIN32)
	find_package(Threads)
endif()

##########
#   GUI
##########

set(FREETYPE_INCLUDE_DIRS CACHE PATH "")
set(FREETYPE_LIBRARIES CACHE PATH "")

set(WXWIDGETS_INCLUDES CACHE PATH "")
set(WXWIDGETS_BASE CACHE PATH "")
set(WXWIDGETS_CORE CACHE PATH "")
set(WXWIDGETS_GL CACHE PATH "")
set(WXWIDGETS_ADV CACHE PATH "")

if(GUI)
	if(FREETYPE_INCLUDE_DIRS STREQUAL "" OR FREETYPE_LIBRARIES STREQUAL "")
		unset(FREETYPE_INCLUDE_DIRS CACHE)
		unset(FREETYPE_LIBRARIES CACHE)
		find_package(Freetype)
	endif()
	
	if(WIN32)
		if(WXWIDGETS_INCLUDES STREQUAL "" OR WXWIDGETS_BASE STREQUAL "" OR WXWIDGETS_CORE STREQUAL "" OR WXWIDGETS_GL STREQUAL "" OR WXWIDGETS_ADV STREQUAL "")
			find_path(WXWIDGETS_INCLUDES build.cfg ${wxWidgets_ROOT_DIR}/include)
			find_library(WXWIDGETS_BASE NAMES wxbase PATHS ${wxWidgets_ROOT_DIR}/lib/*)
			find_library(WXWIDGETS_CORE NAMES wxmsw30u_core_gcc_custom PATHS ${wxWidgets_ROOT_DIR}/lib/)
			find_library(WXWIDGETS_GL NAMES wxmsw30u_gl_gcc_custom PATHS ${wxWidgets_ROOT_DIR}/lib/)
			find_library(WXWIDGETS_ADV NAMES wxmsw30u_gl_gcc_custom PATHS ${wxWidgets_ROOT_DIR}/lib/)
			if(MSVC)
				find_library(WXWIDGETS_PNG NAMES wxmsw30u_png_gcc_custom PATHS ${wxWidgets_ROOT_DIR}/lib/)
				find_library(WXWIDGETS_JPEG NAMES wxmsw30u_jpeg_gcc_custom PATHS ${wxWidgets_ROOT_DIR}/lib/)
				find_library(WXWIDGETS_TIFF NAMES wxmsw30u_tiff_gcc_custom PATHS ${wxWidgets_ROOT_DIR}/lib/)
				find_library(WXWIDGETS_ZLIB NAMES wxmsw30u_zlib_gcc_custom PATHS ${wxWidgets_ROOT_DIR}/lib/)
			endif()
		endif()
	else()
		message("Looking for wxWidgets")
		unset(WXWIDGETS_INCLUDES CACHE)
		unset(WXWIDGETS_BASE CACHE)
		unset(WXWIDGETS_CORE CACHE)
		unset(WXWIDGETS_GL CACHE)
		unset(WXWIDGETS_ADV CACHE)
		find_package(wxWidgets REQUIRED gl core base adv)
		message("Found wxWidgets " ${wxWidgets_FOUND})
		include(${wxWidgets_USE_FILE})
	endif()
endif()