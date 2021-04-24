extern "C" {
#include <time.h> // for clock_gettime
#include <lua5.2/lua.h>
#include <lua5.2/lualib.h>
#include <lua5.2/lauxlib.h>

int timestamp(lua_State *L) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  lua_pushnumber(L, ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
  return 1;
}

int luaopen_luatime(lua_State *L) {
	const luaL_Reg functions[] = {
		{ "timestamp", &timestamp },
		{ NULL, NULL }
	};

	luaL_newlib(L, functions);
	return 1;
}
}
