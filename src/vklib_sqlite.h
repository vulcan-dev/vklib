#ifndef VKLIB_SQLITE_H
#define VKLIB_SQLITE_H

#include "vklib.h"

int sqlite_open(lua_State* L);
int sqlite_close(lua_State* L);
int sqlite_exec(lua_State* L);

#endif // VKLIB_SQLITE_H