set base_path $pwd
set sources_path ${base_path}/libs_mingw/sources
set build_path ${base_path}/libs_mingw/builds

Remove-Item -Recurse ./cmake_build
cmake -B "cmake_build" -G "CodeBlocks - MinGW Makefiles" `
	-DIDE_SETUP=ON `
	-DTASK="Build CLI+GUI" `
	-DINSTALL_PATH="${build_path}/Aether" `
	-DCMAKE_BUILD_TYPE=Release `
	-DFFTW_INCLUDES="${build_path}/fftw" `
	-DFFTW_LIB="${build_path}/fftw/libfftw3-3.dll" `
	-DLUA_LIBRARY="${sources_path}/lua-5.4.4/src/lua54.dll" `
	-DLUA_INCLUDE_DIR="${sources_path}/lua-5.4.4/src" `
	-DPNG_PNG_INCLUDE_DIR="${build_path}/libpng/include" `
	-DPNG_LIBRARY_RELEASE="${build_path}/libpng/bin/libpng16.dll" `
	-DZLIB_LIBRARY_RELEASE="${build_path}/zlib/bin/libzlib.dll" `
	-DCMAKE_PREFIX_PATH="${build_path}/eigen/share/eigen3/cmake;${build_path}/freetype;" `
	-DWXWIDGETS_INCLUDES="${build_path}/wxWidgets/include" `
	-DWXWIDGETS_BASE="${build_path}/wxWidgets/lib/gcc_x64_dll/wxbase32u_gcc_custom.dll" `
	-DWXWIDGETS_CORE="${build_path}/wxWidgets/lib/gcc_x64_dll/wxmsw32u_core_gcc_custom.dll" `
	-DWXWIDGETS_ADV="${build_path}/wxWidgets/lib/gcc_x64_dll/wxmsw32u_adv_gcc_custom.dll" `
	-DWXWIDGETS_GL="${build_path}/wxWidgets/lib/gcc_x64_dll/wxmsw32u_gl_gcc_custom.dll"

$cbp = Get-Content -Path "cmake_build/Aether.cbp" -Raw

$cbp = $cbp.Replace("mingw32-make.exe","mingw32-make.exe -j 8")

Set-Content -Path "cmake_build/Aether.cbp" -Value $cbp