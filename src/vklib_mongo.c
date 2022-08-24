#include "vklib_mongo.h"

#include "mongoc.h"
#include "bson/bson.h"

typedef struct {
    mongoc_client_t* client;
    mongoc_database_t* database;
} client_t;

static const luaL_Reg mt_methods[] = {
    { "new_client", mongo_new_client },

    { "__index", newindex },
    { "__gc", mongo_gc },
    { NULL, NULL }
};

static const luaL_Reg mt_client_methods[] = {
    { "connect", mongo_client_connect },
    { "destroy", mongo_client_destroy },
    { "get_database", mongo_client_get_database },
    { "set_database", mongo_client_set_database },
    { "destroy_database", mongo_client_destroy_database },
    { "get_collection", mongo_client_get_collection },
    { "destroy_collection", mongo_client_destroy_collection },
    { "query", mongoc_query },
    { "__index", newindex },
    { "__gc", mongo_gc },
    { NULL, NULL }
};

static const luaL_Reg mt_database_methods[] = {
    { "destroy", mongo_database_destroy },
    { "__index", newindex2 },
    { "__gc", mongo_gc2 },
    { NULL, NULL }
};

int mongo_new_client(lua_State* L) {
    // setup metatable for client and database
    luaL_newmetatable(L, "vklib.mongo.client");
    luaL_setfuncs(L, mt_client_methods, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, mongo_gc);
    lua_setfield(L, -2, "__gc");

    lua_pop(L, 1);

    // create client
    client_t* client = (client_t*)lua_newuserdata(L, sizeof(client_t));
    client->client = NULL;
    client->database = NULL;
    luaL_getmetatable(L, "vklib.mongo.client");
    lua_setmetatable(L, -2);
    
    return 1;
}

int mongo_client_connect(lua_State* L) {
    client_t* client = luaL_checkudata(L, 1, "vklib.mongo.client");
    const char* uri = luaL_checkstring(L, 2);
    client->client = mongoc_client_new(uri);
    if (client->client == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to connect to MongoDB");
        return 2;
    }

    lua_pushboolean(L, 1);
    return 1;
}

int mongo_client_destroy(lua_State* L) {
    client_t* client = luaL_checkudata(L, 1, "vklib.mongo.client");
    if (client->client != NULL) {
        mongoc_client_destroy(client->client);
        client->client = NULL;
    }
    return 0;
}

int mongo_client_get_database(lua_State* L) {
    // create database metatable
    // luaL_newmetatable(L, "vklib.mongo.database");
    // luaL_setfuncs(L, mt_database_methods, 0);
    // lua_pushvalue(L, -1);
    // lua_setfield(L, -2, "__index");
    // lua_pushcfunction(L, newindex2);
    // lua_setfield(L, -2, "__newindex");
    // lua_pushcfunction(L, mongo_gc2);
    // lua_setfield(L, -2, "__gc");
    // lua_pop(L, 1);

    client_t* client = luaL_checkudata(L, 1, "vklib.mongo.client");
    const char* database_name = luaL_checkstring(L, 2);
    mongoc_database_t* database = mongoc_client_get_database(client->client, database_name);
    if (database == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to get database");
        return 2;
    }

    lua_pushlightuserdata(L, client->database);

    return 1;
}

int mongo_client_set_database(lua_State* L) {
    client_t* client = luaL_checkudata(L, 1, "vklib.mongo.client");
    client->database = lua_touserdata(L, 2);
    return 0;
}

int mongo_client_destroy_database(lua_State* L) {
    client_t* client = luaL_checkudata(L, 1, "vklib.mongo.client");
    if (client->database != NULL) {
        mongoc_database_destroy(client->database);
        client->database = NULL;
    }
    return 0;
}

int mongo_client_get_collection(lua_State* L) {
    client_t* client = luaL_checkudata(L, 1, "vklib.mongo.client");
    const char* collection_name = luaL_checkstring(L, 2);
    const char* database_name = luaL_checkstring(L, 3);
    mongoc_collection_t* collection = mongoc_client_get_collection(client->client, database_name, collection_name);
    if (collection == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to get collection");
        return 2;
    }
    lua_pushlightuserdata(L, collection);
    return 1;
}

int mongo_client_destroy_collection(lua_State* L) {
    client_t* client = luaL_checkudata(L, 1, "vklib.mongo.client");
    mongoc_collection_t* collection = lua_touserdata(L, 2);
    if (collection != NULL) {
        mongoc_collection_destroy(collection);
    }
    return 0;
}

int mongoc_query(lua_State* L) {
    client_t* client = luaL_checkudata(L, 1, "vklib.mongo.client");
    mongoc_collection_t* collection = lua_touserdata(L, 2);
    int flags = luaL_checkinteger(L, 3);
    int skip = luaL_checkinteger(L, 4);
    int limit = luaL_checkinteger(L, 5);
    bson_t* query = lua_touserdata(L, 6);
    bson_t* fields = lua_touserdata(L, 7);
    mongoc_cursor_t* cursor = mongoc_collection_find(collection, flags, skip, limit, 0, query, fields, NULL);
    if (cursor == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to query collection");
        return 2;
    }
    lua_pushlightuserdata(L, cursor);
    return 1;
}

int mongo_database_destroy(lua_State* L) {
    client_t* client = luaL_checkudata(L, 1, "vklib.mongo.database");
    if (client->database != NULL) {
        mongoc_database_destroy(client->database);
        client->database = NULL;
    }
    return 0;
}

// misc
int mongo_gc(lua_State* L) {
    client_t* client = luaL_checkudata(L, 1, "vklib.mongo.client");
    mongo_client_destroy(L);
    return 0;
}

int mongo_gc2(lua_State* L) {
    mongoc_database_t* db = luaL_checkudata(L, 1, "vklib.mongodb.database");
    mongoc_database_destroy(db);
    return 0;
}

int newindex(lua_State* L) {
    return 0;
}
int newindex2(lua_State* L) {
    return 0;
}