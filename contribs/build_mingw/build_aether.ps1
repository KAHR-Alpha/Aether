set base_path $pwd
set sources_path ${base_path}/sources
set build_path ${base_path}/builds

set wxWidgets_ROOT_DIR "${build_path}/wxWidgets"

cd ${sources_path}
if(!(Test-Path Aether_sources.zip))
{
	Invoke-WebRequest -URI https://github.com/KAHR-Alpha/Aether/archive/refs/heads/main.zip -OutFile Aether_sources.zip
}
Expand-Archive -Force ./Aether_sources.zip
cd Aether_sources/Aether-main
Remove-Item -Recurse ./cmake_build
cmake -B "cmake_build" -G "MinGW Makefiles" `
	-DTASK="Build CLI+GUI" `
	-DINSTALL_PATH="${build_path}/Aether" `
	-DCMAKE_BUILD_TYPE=Release `
	-DFFTW_INCLUDES="${build_path}/fftw" `
	-DFFTW_LIB="${build_path}/fftw/libfftw3-3.dll" `
	-DCMAKE_PREFIX_PATH="${build_path}/eigen/share/eigen3/cmake;${build_path}/zlib;${build_path}/libpng;${sources_path}/lua-5.4.4/src;${build_path}/freetype;" `
	-DwxWidgets_ROOT_DIR="${build_path}/wxWidgets" `
	-DWXWIDGETS_BASE="${build_path}/wxWidgets/lib/gcc_x64_dll/wxbase32u_gcc_custom.dll" `
	-DWXWIDGETS_CORE="${build_path}/wxWidgets/lib/gcc_x64_dll/wxmsw32u_core_gcc_custom.dll" `
	-DWXWIDGETS_ADV="${build_path}/wxWidgets/lib/gcc_x64_dll/wxmsw32u_adv_gcc_custom.dll" `
	-DWXWIDGETS_GL="${build_path}/wxWidgets/lib/gcc_x64_dll/wxmsw32u_gl_gcc_custom.dll"
cmake --build "cmake_build" --config Release --parallel 8
cmake --install "cmake_build"
cd ${base_path}
