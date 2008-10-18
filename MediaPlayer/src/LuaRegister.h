#ifndef QSMP_LUAREGISTER
#define QSMP_LUAREGISTER

#include <lua.h>

class LuaTcpSocket;
void registerAll(lua_State* L);

#endif