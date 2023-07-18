set base_path $pwd
set sources_path ${base_path}/sources
set build_path ${base_path}/builds

set wxWidgets_ROOT_DIR "${build_path}/wxWidgets"

cd ${sources_path}
if(!(Test-Path Aether_sources.zip))
{
	Invoke-WebRequest -URI https://github.com/KAHR-Alpha/Aether/archive/refs/heads/main.zip -OutFile Aether_sources.zip
	Expand-Archive -Force ./Aether_sources.zip
}
cd Aether_sources/Aether-main
Remove-Item -Recurse ./cmake_build
cmake -B "cmake_build" -G "Visual Studio 17 2022" `
	-DTASK="Build CLI+GUI" `
	-DINSTALL_PATH="${build_path}/Aether" `
	-DCMAKE_BUILD_TYPE=Release `
	-DFFTW_INCLUDES="${build_path}/fftw" `
	-DFFTW_LIB="${build_path}/fftw/libfftw3-3.lib" `
	-DLUA_LIBRARY="${build_path}/lua/lib/lua.lib" `
	-DLUA_INCLUDE_DIR="${build_path}/lua/include" `
	-DCMAKE_PREFIX_PATH="${build_path}/eigen/share/eigen3/cmake;${build_path}/zlib;${build_path}/libpng;${sources_path}/lua-5.4.4/src;${build_path}/freetype;" `
	-DwxWidgets_ROOT_DIR="${build_path}/wxWidgets" `
	-DWXWIDGETS_INCLUDES="${build_path}/wxWidgets/include" `
	-DWXWIDGETS_BASE="${build_path}/wxWidgets/lib/vc_x64_lib/wxbase32u.lib" `
	-DWXWIDGETS_CORE="${build_path}/wxWidgets/lib/vc_x64_lib/wxmsw32u_core.lib" `
	-DWXWIDGETS_ADV="${build_path}/wxWidgets/lib/vc_x64_lib/wxmsw32u_adv.lib" `
	-DWXWIDGETS_PNG="${build_path}/wxWidgets/lib/vc_x64_lib/wxpng.lib" `
	-DWXWIDGETS_JPEG="${build_path}/wxWidgets/lib/vc_x64_lib/wxjpeg.lib" `
	-DWXWIDGETS_TIFF="${build_path}/wxWidgets/lib/vc_x64_lib/wxtiff.lib" `
	-DWXWIDGETS_ZLIB="${build_path}/wxWidgets/lib/vc_x64_lib/wxzlib.lib" `
	-DWXWIDGETS_GL="${build_path}/wxWidgets/lib/vc_x64_lib/wxmsw32u_gl.lib"
cmake --build "cmake_build" --config Release --parallel 8
cmake --install "cmake_build"
cd ${base_path}
