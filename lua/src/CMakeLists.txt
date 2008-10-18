project(lua)
set( sources lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c
	lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c ltm.c
	lundump.c lvm.c lzio.c
	lauxlib.c lbaselib.c ldblib.c liolib.c lmathlib.c loslib.c ltablib.c
	lstrlib.c loadlib.c linit.c)
set(lua_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "lua include dir")

add_library(lua STATIC ${sources})

