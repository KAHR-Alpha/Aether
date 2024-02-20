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
	-DPNG_LIBRARY_RELEASE="${build_path}/libpng/lib/libpng16_staticd.lib" `
	-DZLIB_LIBRARY_RELEASE="${build_path}/zlib/lib/zlibstaticd.lib" `
	-DCMAKE_PREFIX_PATH="${build_path}/eigen/share/eigen3/cmake;${sources_path}/lua-5.4.4/src;${build_path}/freetype;" `
	-DWXWIDGETS_INCLUDES="${build_path}/wxWidgets/include" `
	-DWXWIDGETS_BASE="${build_path}/wxWidgets/lib/vc_x64_lib/wxbase32ud.lib" `
	-DWXWIDGETS_CORE="${build_path}/wxWidgets/lib/vc_x64_lib/wxmsw32ud_core.lib" `
	-DWXWIDGETS_ADV="${build_path}/wxWidgets/lib/vc_x64_lib/wxmsw32ud_adv.lib" `
	-DWXWIDGETS_PNG="${build_path}/wxWidgets/lib/vc_x64_lib/wxpngd.lib" `
	-DWXWIDGETS_JPEG="${build_path}/wxWidgets/lib/vc_x64_lib/wxjpegd.lib" `
	-DWXWIDGETS_TIFF="${build_path}/wxWidgets/lib/vc_x64_lib/wxtiffd.lib" `
	-DWXWIDGETS_ZLIB="${build_path}/wxWidgets/lib/vc_x64_lib/wxzlibd.lib" `
	-DWXWIDGETS_GL="${build_path}/wxWidgets/lib/vc_x64_lib/wxmsw32ud_gl.lib"
