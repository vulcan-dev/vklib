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
// #include "sqlite3.h"
static const luaL_Reg sqlite_funcs[] = {
    { "open", sqlite_open },
    { "close", sqlite_close },
    { "exec", sqlite_exec },
    { NULL, NULL }
};
#endif

#ifdef VKLIB_MONGODB
#include "vklib_mongo.h"
#include "mongoc.h"
static const luaL_Reg mongodb_funcs[] = {
    { "new_client", mongo_new_client },
    //{ "destroy_client", mongo_destroy_client },
    //{ "get_database", mongo_get_database },
    //{ "destroy_database", mongo_destroy_database },
    { NULL, NULL }
};
#endif

int gc_destroy(lua_State* L) {
    #ifdef VKLIB_MONGODB
    mongoc_cleanup();
    #endif
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

    #ifdef VKLIB_MONGODB
    luaL_newlib(L, mongodb_funcs);
    lua_setfield(L, -2, "mongodb");
    #endif

    lua_newtable(L);
    lua_pushcfunction(L, gc_destroy);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);

    return 1;
}

void Segfault_Handler(int signo) {
     fprintf(stderr,"\n[!] Oops! Segmentation fault...\n");
}

VKLIB_API int luaopen_vklib(lua_State* L) {
    signal(SIGSEGV, Segfault_Handler);

#ifdef VKLIB_MONGODB
    mongoc_init();
#endif

    base_open(L);
    return 1;
}