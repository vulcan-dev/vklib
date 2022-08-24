#ifndef VKLIB_MONGO_H
#define VKLIB_MONGO_H

#include "vklib.h"

int mongo_new_client(lua_State* L);
int mongo_get_database(lua_State* L);
int mongo_destroy_database(lua_State* L);
int mongo_destroy_client(lua_State* L);

#endif // VKLIB_MONGO_H