#include "vklib.h"
#include "vklib_fs.h"

static const luaL_Reg fs_funcs[] = {
    { "get_cwd", fs_getcwd },
    { "create_dir", fs_create_dir },
    { "remove_dir", fs_remove_dir },
    { "create_file", fs_create_file },
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

    return 1;
}

VKLIB_API int luaopen_vklib(lua_State* L) {
    base_open(L);
    return 1;
}