#ifndef VKLIB_MONGO_H
#define VKLIB_MONGO_H

#include "vklib.h"

// general
int mongo_new_client(lua_State* L);

// client
int mongo_client_connect(lua_State* L);
int mongo_client_destroy(lua_State* L);
int mongo_client_get_database(lua_State* L);
int mongo_client_set_database(lua_State* L);
int mongo_client_destroy_database(lua_State* L);
int mongo_client_get_collection(lua_State* L);
int mongo_client_destroy_collection(lua_State* L);
int mongoc_query(lua_State* L);

// database
int mongo_database_connect(lua_State* L);
int mongo_database_destroy(lua_State* L);

// mt
int newindex(lua_State* L);
int newindex2(lua_State* L);
int mongo_gc(lua_State* L);
int mongo_gc2(lua_State* L);

#endif // VKLIB_MONGO_H