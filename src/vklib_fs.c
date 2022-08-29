#include "vklib_fs.h"
#include "vklib_compat.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#ifdef _WIN32 // Windows
#include "dirent_compat.h"
#include <windows.h>
#include <tchar.h>

#define getcwd _getcwd
#define mkdir(path) _mkdir(path)
#define rmdir _rmdir

#else // Linux
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#endif

/*-------------------------------------------------------------------------*\
* Definitions
\*-------------------------------------------------------------------------*/
#ifndef S_ISREG
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif

#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

/*-------------------------------------------------------------------------*\
* Compatibility functions
\*-------------------------------------------------------------------------*/
int _compat_mkdir(const char* path, struct stat* st) {
    #ifdef _WIN32
    if (mkdir(path) == 0) {
        return 0;
    } else {
        return -1;
    }
    #else
        if (stat(path, &st) == -1) {
            mkdir(path, 0700);
            return 0;
        } else {
            return -1;
        }
    #endif
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

    struct stat st = {0};
    if (overwrite) {
        if (_compat_rmdir(path, 1) != 0) {
            if (errno != ENOENT) {
                lua_pushnil(L);
                lua_pushstring(L, get_error("rmdir", errno));
                return 2;
            }
        }
        
        if (_compat_mkdir(path, &st) != 0) {
            lua_pushnil(L);
            lua_pushstring(L, get_error("mkdir", errno));
            return 2;
        }
    } else {
        if (_compat_mkdir(path, &st) != 0) {
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
    * Checks if a directory exists
    * @param path - the path of the directory to check
    * @return true if the directory exists, nil & error message otherwise
*/
int fs_dir_exists(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    struct stat buf;
    int exists = stat(path, &buf) == 0;
    lua_pushboolean(L, exists);
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

    // get path, filename and extension
    char* path_copy = strdup(path);
    char* filename = (char*)malloc(strlen(path) + 1);
    char* slash = strrchr(path_copy, '/');
    if (slash == NULL) {
        strcpy(filename, path_copy);
        strcpy(path_copy, ".");
    } else {
        strcpy(filename, slash + 1);
        *slash = '\0';
    }
    char* dir = (char*)malloc(strlen(path_copy) + 1);
    strcpy(dir, path_copy);
    free(path_copy);

    struct stat buf;
    if (stat(dir, &buf) != 0) {
        if (_compat_mkdir(dir, S_IWUSR) != 0) {
            lua_pushnil(L);
            lua_pushstring(L, get_error("mkdir", errno));
            return 2;
        }
    }
    free(dir);

    FILE* file = fopen(path, "w");
    if (file == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, get_error("fopen", errno));
        return 2;
    }
    fwrite(contents, 1, strlen(contents), file);
    fclose(file);

    lua_pushboolean(L, 1);
    return 1;
}

/*
    * Removes a file
    * @param path - the path of the file to remove
    * @return true if successful, nil & error message otherwise
*/
int fs_remove_file(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    if (remove(path) != 0) {
        lua_pushnil(L);
        lua_pushstring(L, get_error("remove", errno));
        return 2;
    }
    lua_pushboolean(L, 1);
    return 1;
}

/*
    * Checks if a file exists
    * @param path - the path of the file to check
    * @return true if the file exists, nil & error message otherwise
*/
int fs_file_exists(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    int exists = is_file(path);

    lua_pushboolean(L, exists);
    return 1;
}

/*
    * Reads a file
    * @param path - the path of the file to read
    * @return the contents of the file if successful, nil & error message otherwise
*/
int fs_read_file(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, get_error("fopen", errno));
        return 2;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* contents = malloc(size + 1);
    if (contents == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, get_error("malloc", errno));
        return 2;
    }
    fread(contents, 1, size, file);
    fclose(file);
    contents[size] = '\0';

    lua_pushstring(L, contents);
    free(contents);
    return 1;
}

/*
    * Returns a table of files in a directory
    * @param path - the path of the directory to get files from
    * @return a table of files if successful, nil & error message otherwise
    * @note This function is not recursive
*/
int fs_get_files(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    DIR* dir = opendir(path);
    if (dir == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, get_error("opendir", errno));
        return 2;
    }
    struct dirent* entry;
    lua_newtable(L);
    int i = 1;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') {
            lua_pushstring(L, entry->d_name);
            lua_rawseti(L, -2, i);
            i++;
        }
    }
    closedir(dir);
    return 1;
}