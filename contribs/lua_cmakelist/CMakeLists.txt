cmake_minimum_required(VERSION 3.25)

project(Lua C)

set(core_src src/lapi.c
			 src/lcode.c
			 src/lctype.c
			 src/ldebug.c
			 src/ldo.c
			 src/ldump.c
			 src/lfunc.c
			 src/lgc.c
			 src/llex.c
			 src/lmem.c
			 src/lobject.c
			 src/lopcodes.c
			 src/lparser.c
			 src/lstate.c
			 src/lstring.c
			 src/ltable.c
			 src/ltm.c
			 src/lundump.c
			 src/lvm.c
			 src/lzio.c)

set(lib_src src/lauxlib.c
			src/lbaselib.c
			src/lcorolib.c
			src/ldblib.c
			src/liolib.c
			src/lmathlib.c
			src/loadlib.c
			src/loslib.c
			src/lstrlib.c
			src/ltablib.c
			src/lutf8lib.c
			src/linit.c)

set(base_src ${core_src} ${lib_src})

set(lua_src src/lua.c)
set(luac_src src/luac.c)

set(CMAKE_C_FLAGS "-DLUA_COMPAT_5_3" CACHE STRING "" FORCE)

add_library(lua STATIC ${base_src})
target_include_directories(lua PUBLIC src)

set(lib_headers src/lua.h src/luaconf.h src/lualib.h src/lauxlib.h src/lua.hpp)

install(FILES ${lib_headers} DESTINATION ./include)
install(TARGETS lua LIBRARY)
