#include "vklib_sqlite.h"

#include <stdio.h>
#include "sqlite3.h"

int sqlite_connect(lua_State* L) {
	sqlite3* db;
	const char* db_name = luaL_checkstring(L, 1);
	int rc = sqlite3_open(db_name, &db);
	if (rc) {
		lua_pushstring(L, sqlite3_errmsg(db));
		sqlite3_close(db);
		return 1;
	}
	
	printf("\n[+] Connected to database: %s\n", db_name);
	sqlite3_close(db);

	return 0;
}