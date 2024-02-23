set base_path $pwd
set sources_path ${base_path}/libs_mingw/sources
set build_path ${base_path}/libs_mingw/builds

mkdir ${sources_path}
mkdir ${build_path}

############
#   eigen
############

cd ${sources_path}
If(!(Test-Path eigen-3.4.0.zip))
{
	Invoke-WebRequest -URI https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.zip -OutFile eigen-3.4.0.zip
	Expand-Archive -Force ./eigen-3.4.0.zip
}
cd eigen-3.4.0/eigen-3.4.0
cmake -B "cmake_build" -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX="${build_path}/eigen" -DCMAKE_BUILD_TYPE=Release
cmake --build "cmake_build" -- config Release --parallel 8
cmake --install "cmake_build"


###########
#   zlib
###########

cd ${sources_path}
if(!(Test-Path zlib131.zip))
{
	Invoke-WebRequest -URI https://zlib.net/zlib131.zip -OutFile zlib131.zip
	Expand-Archive -Force ./zlib131.zip
}
cd zlib131/zlib-1.3.1
cmake -B "cmake_build" -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX="${build_path}/zlib" -DCMAKE_BUILD_TYPE=Release
cmake --build "cmake_build" --config Release --parallel 8
cmake --install "cmake_build"


#############
#   libpng
#############

cd ${sources_path}
if(!(Test-Path libpng.zip))
{
	Invoke-WebRequest -UserAgent "Wget" -URI https://sourceforge.net/projects/libpng/files/latest/download -OutFile libpng.zip
	Expand-Archive -Force ./libpng.zip
}
cd libpng/lpng1643
cmake -B "cmake_build" -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX="${build_path}/libpng" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="../../zlib_build"
cmake --build "cmake_build" --config Release --parallel 8
cmake --install "cmake_build"


###############
#   Freetype
###############

cd ${sources_path}
if(!(Test-Path freetype.zip))
{
	Invoke-WebRequest -UserAgent "Wget" -URI https://sourceforge.net/projects/freetype/files/latest/download -OutFile freetype.zip
	Expand-Archive -Force ./freetype.zip
}
cd freetype/freetype-2.13.2
cmake -B "cmake_build" -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX="${build_path}/freetype" -DCMAKE_BUILD_TYPE=Release `
	-DFT_DISABLE_HARFBUZZ=ON `
	-DFT_DISABLE_BZIP2=ON `
	-DFT_DISABLE_PNG=ON `
	-DFT_DISABLE_ZLIB=ON 
cmake --build "cmake_build" --config Release --parallel 8
cmake --install "cmake_build"
cd ${base_path}


##########
#   Lua
##########

cd ${sources_path}
if(!(Test-Path lua-5.4.4.tar.gz))
{
	Invoke-WebRequest -URI https://www.lua.org/ftp/lua-5.4.4.tar.gz -OutFile lua-5.4.4.tar.gz
}
tar -xzf lua-5.4.4.tar.gz
cd lua-5.4.4
mingw32-make mingw


############
#   FFTW3
############

cd ${build_path}
if(!(Test-Path fftw.zip))
{
	Invoke-WebRequest -URI https://fftw.org/pub/fftw/fftw-3.3.5-dll64.zip -OutFile fftw.zip
Expand-Archive -Force ./fftw.zip
}


################
#   wxWidgets
################

cd ${sources_path}
if(!(Test-Path wxWidgets-3.2.2.1.zip))
{
	Invoke-WebRequest -URI https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.2.1/wxWidgets-3.2.2.1.zip -OutFile wxWidgets-3.2.2.1.zip
	Expand-Archive -Force ./wxWidgets-3.2.2.1.zip
}
cd wxWidgets-3.2.2.1
cmake -B "cmake_build" -G "MinGW Makefiles"  -DCMAKE_INSTALL_PREFIX="${build_path}/wxWidgets" -DCMAKE_BUILD_TYPE=Release
cmake --build "cmake_build" --config Release --parallel 8
cmake --install "cmake_build"

cd ${base_path}

Copy-Item -Force -Path "${build_path}/wxWidgets/lib/gcc_x64_dll/mswu/wx/setup.h" -Destination "${build_path}/wxWidgets/include/wx/setup.h"