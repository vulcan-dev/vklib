#include "vklib_sqlite.h"

#include <stdio.h>
#include "sqlite3.h"

int sqlite_open(lua_State* L) {
	sqlite3* db;
	const char* db_name = luaL_checkstring(L, 1);
	int rc = sqlite3_open(db_name, &db);
	if (rc) {
		lua_pushnil(L);
		lua_pushstring(L, sqlite3_errmsg(db));
		sqlite3_close(db);
		return 2;
	}
	
	lua_pushlightuserdata(L, db);
	return 1;
}

int sqlite_close(lua_State* L) {
	sqlite3* db = lua_touserdata(L, 1);
	sqlite3_close(db);
	return 0;
}

int sqlite_exec(lua_State* L) {
	sqlite3* db = lua_touserdata(L, 1);
	const char* query = luaL_checkstring(L, 2);
	char* errmsg = 0;

	int i = 1;
	char** db_result;
	int n_rows, n_cols;
	int rc = sqlite3_get_table(db, query, &db_result, &n_rows, &n_cols, &errmsg);
	if (rc != SQLITE_OK) {
		lua_pushnil(L);
		lua_pushstring(L, errmsg);
		sqlite3_free(errmsg);
		return 2;
	}

	// check if there are any results
	if (n_rows == 0) {
		lua_pushnil(L);
		return 1;
	}

	// check if we're selecting only one row (Example: "SELECT id FROM players WHERE name='person'")
	if (n_rows == 1 && n_cols == 1) {
		lua_pushstring(L, db_result[1]);
		sqlite3_free_table(db_result);
		return 1;
	}

	// we have multiple rows, return everything :)
	lua_newtable(L);
	for (int i = 0; i < n_rows; i++) {
		lua_newtable(L);

		for (int j = 0; j < n_cols; j++) {
			lua_pushstring(L, db_result[j]);
			lua_pushstring(L, db_result[n_cols + i * n_cols + j]);
			lua_settable(L, -3);
		}
		lua_rawseti(L, -2, i + 1);
	}

	sqlite3_free_table(db_result);
	return 1;
}