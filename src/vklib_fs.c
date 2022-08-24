#include "vklib_fs.h"
#include "vklib_compat.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32 // Windows
#include "dirent.h"
#include <direct.h>
#include <sys/stat.h>
#include <windows.h>
#include <tchar.h>
#include <string.h>

#define getcwd _getcwd
#define mkdir(path) _mkdir(path)
#define rmdir _rmdir

#else // Linux
#include <dirent.h>
#include <unistd.h>
#endif

/*-------------------------------------------------------------------------*\
* Definitions
\*-------------------------------------------------------------------------*/
#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif

#define S_IRUSR 0400
#define S_IREAD S_IRUSR
#define S_IWUSR 0200
#define S_IWRITE S_IWUSR
#define S_IXUSR 0100
#define S_IRWXU (S_IREAD | S_IWRITE | S_IXUSR)
#define S_IRGRP 0040
#define S_IWGRP 0020
#define S_IXGRP 0010
#define S_IRWXG (S_IRGRP | S_IWGRP | S_IXGRP)
#define S_IROTH 0004
#define S_IXOTH 0001

/*-------------------------------------------------------------------------*\
* Compatibility functions
\*-------------------------------------------------------------------------*/
int _compat_mkdir(const char* path, int mode) {
    if (mkdir(path) == 0) {
        return 0;
    } else {
        return -1;
    }
}

char is_file(const char* path) {
    struct stat buf;
    if (stat(path, &buf) != 0) {
        return 0;
    }

    return S_ISREG(buf.st_mode);
}

char _compat_rmdir(const char* path, char recurse) {
    if (recurse == 0) {
        return rmdir(path);
    }

    DIR* dir = opendir(path);
    if (dir == NULL) {
        return -1;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        char* subpath = malloc(strlen(path) + strlen(entry->d_name) + 2);
        if (subpath == NULL) {
            closedir(dir);
            return -1;
        }
        sprintf(subpath, "%s/%s", path, entry->d_name);
        if (is_file(subpath)) {
            remove(subpath);
        } else {
            _compat_rmdir(subpath, 1);
        }
        free(subpath);
    }
    closedir(dir);

    return rmdir(path);
}

/*-------------------------------------------------------------------------*\
* Filesystem functions
\*-------------------------------------------------------------------------*/

/*
    * Returns the current working directory
*/
int fs_getcwd(lua_State* L) {
    char buf[1024];
    if (getcwd(buf, sizeof(buf)) != NULL) {
        lua_pushstring(L, buf);
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushstring(L, get_error("getcwd", errno));
        return 2;
    }
}

/*
    * Creates a directory
    * @param path - the path of the directory to create
    * @param overwrite - whether to overwrite an existing directory
    * @return true if successful, nil & error message otherwise
*/
int fs_create_dir(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
	char overwrite = lua_toboolean(L, 2);

    if (overwrite) {
        if (_compat_rmdir(path, 1) != 0) {
            if (errno != ENOENT) {
                lua_pushnil(L);
                lua_pushstring(L, get_error("rmdir", errno));
                return 2;
            }
        }
        
        if (_compat_mkdir(path, S_IRWXU) != 0) {
            lua_pushnil(L);
            lua_pushstring(L, get_error("mkdir", errno));
            return 2;
        }
    } else {
        if (_compat_mkdir(path, S_IRWXU) != 0) {
            lua_pushnil(L);
            lua_pushstring(L, get_error("mkdir", errno));
            return 2;
        }
    }

    lua_pushboolean(L, 1);
    return 1;
}

/*
    * Removes a directory
    * @param path - the path of the directory to remove
    * @return true if successful, nil & error message otherwise
*/
int fs_remove_dir(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    const char recurse = lua_toboolean(L, 2);

    if (_compat_rmdir(path, recurse) != 0) {
        lua_pushnil(L);
        lua_pushstring(L, get_error("rmdir", errno));
        return 2;
    }
    lua_pushboolean(L, 1);
    return 1;
}

/*
    * Creates a file
    * @param path - the path of the file to create
    * @param contents - the contents of the file (optional)
    * @param mode - the mode to create the file with (optional)
    * @return true if successful, nil & error message otherwise
*/
int fs_create_file(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    const char* contents = luaL_optstring(L, 2, "");
    const char* mode = luaL_optstring(L, 3, "w");

    FILE* file = fopen(path, mode);
    if (file == NULL) {
        file = fopen(path, "w");
        if (file == NULL) {
            lua_pushnil(L);
            lua_pushstring(L, get_error("fopen", errno));
            return 2;
        }
    }
    fwrite(contents, 1, strlen(contents), file);
    fclose(file);

    lua_pushboolean(L, 1);
    return 1;
}