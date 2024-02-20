set base_path $pwd
set sources_path ${base_path}/libs_vs22/sources
set build_path ${base_path}/libs_vs22/builds

set wxWidgets_ROOT_DIR "${build_path}/wxWidgets"

# Remove-Item -Recurse ./cmake_build

cmake -B "cmake_build_VS" -G "Visual Studio 17 2022" `
	-DIDE_SETUP=ON `
	-DTASK="Build CLI+GUI" `
	-DINSTALL_PATH="${build_path}/Aether" `
	-DDEVTESTS=1 `
	-DFFTW_INCLUDES="${build_path}/fftw" `
	-DFFTW_LIB="${build_path}/fftw/libfftw3-3.lib" `
	-DLUA_LIBRARY="${build_path}/lua/lib/lua.lib" `
	-DLUA_INCLUDE_DIR="${build_path}/lua/include" `
	-DPNG_PNG_INCLUDE_DIR="${build_path}/libpng/include" `
	-DPNG_LIBRARY_RELEASE="${build_path}/libpng/lib/libpng16_static.lib" `
	-DZLIB_LIBRARY_RELEASE="${build_path}/zlib/lib/zlibstatic.lib" `
	-DCMAKE_PREFIX_PATH="${build_path}/eigen/share/eigen3/cmake;${sources_path}/lua-5.4.4/src;${build_path}/freetype;" `
	-DWXWIDGETS_INCLUDES="${build_path}/wxWidgets/include" `
	-DWXWIDGETS_BASE="${build_path}/wxWidgets/lib/vc_x64_lib/wxbase32u.lib" `
	-DWXWIDGETS_CORE="${build_path}/wxWidgets/lib/vc_x64_lib/wxmsw32u_core.lib" `
	-DWXWIDGETS_ADV="${build_path}/wxWidgets/lib/vc_x64_lib/wxmsw32u_adv.lib" `
	-DWXWIDGETS_PNG="${build_path}/wxWidgets/lib/vc_x64_lib/wxpng.lib" `
	-DWXWIDGETS_JPEG="${build_path}/wxWidgets/lib/vc_x64_lib/wxjpeg.lib" `
	-DWXWIDGETS_TIFF="${build_path}/wxWidgets/lib/vc_x64_lib/wxtiff.lib" `
	-DWXWIDGETS_ZLIB="${build_path}/wxWidgets/lib/vc_x64_lib/wxzlib.lib" `
	-DWXWIDGETS_GL="${build_path}/wxWidgets/lib/vc_x64_lib/wxmsw32u_gl.lib"
