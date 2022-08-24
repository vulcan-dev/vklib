#ifndef VKLIB_FS_H
#define VKLIB_FS_H

#include "vklib.h"

int fs_getcwd(lua_State* L);
int fs_create_dir(lua_State* L);
int fs_remove_dir(lua_State* L);
int fs_dir_exists(lua_State* L);

int fs_create_file(lua_State* L);
int fs_remove_file(lua_State* L);
int fs_file_exists(lua_State* L);

#endif