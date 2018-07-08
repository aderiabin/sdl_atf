#include "qt_impl/lua_lib/sdl_remote_adapter_qt_client_lua_wrapper.h"
#include "lua_lib/sdl_remote_adapter_client_lua_wrapper.h"

extern "C" {

int luaopen_remote(lua_State *L) {
  static const luaL_Reg library_functions[] = {
      {"RemoteMqClient",
       lua_lib::SDLRemoteClientLuaWrapper::create_SDLRemoteMqClient},
      {"RemoteTestAdapter",
       lua_lib::SDLRemoteTestAdapterLuaWrapper::create_SDLRemoteTestAdapter},
      {NULL, NULL}};

  luaL_newlib(L, library_functions);
  return 1;
}

}
