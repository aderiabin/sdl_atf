
extern "C" {
#include <lua5.2/lua.h>
#include <lua5.2/lualib.h>
#include <lua5.2/lauxlib.h>
}

namespace lua_lib {
struct SDLRemoteTestAdapterLuaWrapper {
  static int create_SDLRemoteTestAdapter(lua_State* L);
  static int destroy_SDLRemoteTestAdapter(lua_State* L);
  static void registerSDLRemoteTestAdapter(lua_State* L);
  static class SDLRemoteTestAdapterClient* get_instance(lua_State* L);

  static int lua_connected(lua_State* L);
  static int lua_open(lua_State* L);
  static int lua_open_with_params(lua_State* L);
  static int lua_send(lua_State* L);
  static int lua_receive(lua_State* L);
  static int lua_clear(lua_State* L);
};
}
