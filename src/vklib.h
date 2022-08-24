#ifndef VKLIB_H
#define VKLIB_H

#define VKLIB_VERSION       "VKLib 1.0.0"
#define VKLIB_COPYRIGHT     "Copyright (C) 2022 Daniel Wakefield"

#ifdef _WIN32
#define VKLIB_API __declspec(dllexport)
#else
#define VKLIB_API __attribute__((visibility("default")))
#endif

#include "lua.h"
#include "lauxlib.h"

#include <time.h>
#include <signal.h>

#define TIMER_START() clock_t start = clock()
#define TIMER_END() ({ clock_t end = clock(); double elapsed = (double)(end - start) / CLOCKS_PER_SEC; elapsed; printf("ms: %d, nsec: %d\n", (int)(elapsed * 1000), (int)(elapsed * 1000000)); })

VKLIB_API int luaopen_vklib(lua_State *L);

#endif // VKLIB_H