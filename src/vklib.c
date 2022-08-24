#include "vklib.h"

#ifdef VKLIB_FILESYSTEM
#include "vklib_fs.h"
static const luaL_Reg fs_funcs[] = {
    { "get_cwd", fs_getcwd },
    { "create_dir", fs_create_dir },
    { "dir_exists", fs_dir_exists },
    { "remove_file", fs_remove_file },
    { "file_exists", fs_file_exists },
    { "remove_dir", fs_remove_dir },
    { "create_file", fs_create_file },
    { NULL, NULL }
};
#endif

#ifdef VKLIB_REQUESTS
#include "vklib_requests.h"
static const luaL_Reg request_funcs[] = {
    { "get", request_get },
    { NULL, NULL }
};
#endif

#ifdef VKLIB_SQLITE
#include "vklib_sqlite.h"
static const luaL_Reg sqlite_funcs[] = {
    { "open", sqlite_open },
    { "close", sqlite_close },
    { "exec", sqlite_exec },
    { NULL, NULL }
};
#endif

static int gc_destroy(lua_State* L) {
    return 0;
}

static int base_open(lua_State* L) {
    lua_newtable(L);
    lua_pushstring(L, VKLIB_VERSION);
    lua_setfield(L, -2, "_VERSION");
    lua_pushstring(L, VKLIB_COPYRIGHT);
    lua_setfield(L, -2, "_COPYRIGHT");

    #ifdef VKLIB_FILESYSTEM
    luaL_newlib(L, fs_funcs);
    lua_setfield(L, -2, "fs");
    #endif

    #ifdef VKLIB_REQUESTS
    luaL_newlib(L, request_funcs);
    lua_setfield(L, -2, "request");
    #endif

    #ifdef VKLIB_SQLITE
    luaL_newlib(L, sqlite_funcs);
    lua_setfield(L, -2, "sqlite");
    #endif

    lua_newtable(L);
    lua_pushcfunction(L, gc_destroy);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);

    return 1;
}

#ifdef VKLIB_MONGO
#include "vklib_mongo/common.h"

static int f_type(lua_State *L) {
	luaL_checkany(L, 1);
	lua_pushstring(L, typeName(L, 1));
	return 1;
}

static const luaL_Reg funcs[] = {
	{"type", f_type},
	{"binary", newBinary},
	{"bson", newBSON},
	{"client", newClient},
	{"date_time", newDateTime},
	{"decimal128", newDecimal128},
	{"double", newDouble},
	{"int32", newInt32},
	{"int64", newInt64},
	{"javascript", newJavascript},
	{"objectid", newObjectID},
	{"readprefs", newReadPrefs},
	{"regex", newRegex},
	{"timestamp", newTimestamp},
	{0, 0}
};

char NEW_BINARY, NEW_DATETIME, NEW_DECIMAL128, NEW_JAVASCRIPT, NEW_REGEX, NEW_TIMESTAMP;
char GLOBAL_MAXKEY, GLOBAL_MINKEY, GLOBAL_NULL;

static int mongo_open(lua_State* L) {
#if LUA_VERSION_NUM < 502
	luaL_register(L, "mongo", funcs);
#else
	luaL_newlib(L, funcs);
    lua_setfield(L, -2, "mongo");
#endif

	/* Cache BSON type constructors for quick access */
	lua_getfield(L, -1, "Binary");
	lua_rawsetp(L, LUA_REGISTRYINDEX, &NEW_BINARY);
	lua_getfield(L, -1, "DateTime");
	lua_rawsetp(L, LUA_REGISTRYINDEX, &NEW_DATETIME);
	lua_getfield(L, -1, "Decimal128");
	lua_rawsetp(L, LUA_REGISTRYINDEX, &NEW_DECIMAL128);
	lua_getfield(L, -1, "Javascript");
	lua_rawsetp(L, LUA_REGISTRYINDEX, &NEW_JAVASCRIPT);
	lua_getfield(L, -1, "Regex");
	lua_rawsetp(L, LUA_REGISTRYINDEX, &NEW_REGEX);
	lua_getfield(L, -1, "Timestamp");
	lua_rawsetp(L, LUA_REGISTRYINDEX, &NEW_TIMESTAMP);

	/* Create BSON type singletons */
	pushMaxKey(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, -3, "MaxKey");
	lua_rawsetp(L, LUA_REGISTRYINDEX, &GLOBAL_MAXKEY);
	pushMinKey(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, -3, "MinKey");
	lua_rawsetp(L, LUA_REGISTRYINDEX, &GLOBAL_MINKEY);
	pushNull(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, -3, "Null");
	lua_rawsetp(L, LUA_REGISTRYINDEX, &GLOBAL_NULL);

	mongoc_init();
	return 1;
}
#endif

void Segfault_Handler(int signo) {
     fprintf(stderr,"\n[!] Oops! Segmentation fault...\n");
}

VKLIB_API int luaopen_vklib(lua_State* L) {
    signal(SIGSEGV, Segfault_Handler);

    base_open(L);

#ifdef VKLIB_MONGO
    mongoc_init();
    mongo_open(L);
#endif
    return 1;
}