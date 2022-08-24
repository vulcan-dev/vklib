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

#define TIME_MONITORING

#ifdef TIME_MONITORING
#define TIMER_START() clock_t start = clock();
#define TIMER_END() printf("[%s] ms: %.2f | ns: %.2f\n", __FUNCTION__, (double)(clock() - start) / CLOCKS_PER_SEC * 1000, (double)(clock() - start) * 1000000 / CLOCKS_PER_SEC);
#else
#define TIMER_START()
#define TIMER_END()
#endif

VKLIB_API int luaopen_vklib(lua_State *L);

#endif // VKLIB_H