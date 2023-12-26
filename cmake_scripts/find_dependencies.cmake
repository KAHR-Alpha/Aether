
############
#   Eigen
############

find_package(Eigen3 REQUIRED)

###########
#   FFTW
###########

find_path(FFTW_INCLUDES fftw3.h /usr/lib /usr/local/include libs libs/fftw64)
find_library(FFTW_LIB NAMES fftw3-3 fftw3 PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib libs libs/fftw64)
# find_package(FFTW3)


##########
#   Lua
##########

find_package(Lua)


#############
#   Libpng
#############

find_package(PNG)

###########
#   zlib
###########

find_package(ZLIB)
	
if(NOT WIN32)
	find_package(Threads)
endif()

##########
#   GUI
##########

if(GUI)
	find_package(Freetype)
	
	if(WIN32)
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
	else()
		message("Looking for wxWidgets")
		find_package(wxWidgets REQUIRED gl core base adv)
		message("Found wxWidgets " ${wxWidgets_FOUND})
		include(${wxWidgets_USE_FILE})
	endif()
endif()