#include "vklib_requests.h"

int request_get(lua_State* L) {
	lua_pushstring(L, "Hello, World!");
	return 1;
}