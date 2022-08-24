#include "vklib.h"
#include "vklib_fs.h"
#include "vklib_requests.h"
#include "vklib_sqlite.h"

#include "sqlite3.h"

static const luaL_Reg fs_funcs[] = {
    { "get_cwd", fs_getcwd },
    { "create_dir", fs_create_dir },
    { "remove_dir", fs_remove_dir },
    { "create_file", fs_create_file },
    { NULL, NULL }
};

static const luaL_Reg request_funcs[] = {
    { "get", request_get },
    { NULL, NULL }
};

static const luaL_Reg sqlite_funcs[] = {
    { "open", sqlite_open },
    { "close", sqlite_close },
    { "exec", sqlite_exec },
    { NULL, NULL }
};

static int base_open(lua_State* L) {
    lua_newtable(L);
    lua_pushstring(L, VKLIB_VERSION);
    lua_setfield(L, -2, "_VERSION");
    lua_pushstring(L, VKLIB_COPYRIGHT);
    lua_setfield(L, -2, "_COPYRIGHT");

    luaL_newlib(L, fs_funcs);
    lua_setfield(L, -2, "fs");

    luaL_newlib(L, request_funcs);
    lua_setfield(L, -2, "request");

    luaL_newlib(L, sqlite_funcs);
    lua_setfield(L, -2, "sqlite");

    return 1;
}

void Segfault_Handler(int signo) {
     fprintf(stderr,"\n[!] Oops! Segmentation fault...\n");
}

VKLIB_API int luaopen_vklib(lua_State* L) {
    signal(SIGSEGV, Segfault_Handler);

    base_open(L);
    return 1;
}