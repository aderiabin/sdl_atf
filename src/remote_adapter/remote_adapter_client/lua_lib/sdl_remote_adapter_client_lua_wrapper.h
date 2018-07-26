#pragma once

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace lua_lib {
struct SDLRemoteClientLuaWrapper {
  static int create_SDLRemoteMqClient(lua_State* L);
  static int destroy_SDLRemoteMqClient(lua_State* L);
  static void registerSDLRemoteMqClient(lua_State* L);
  static class SDLRemoteTestAdapterClient* get_instance(lua_State* L);

  static int lua_connected(lua_State* L);

  static int lua_open(lua_State* L);
  static int lua_open_with_params(lua_State* L);
  static int lua_send(lua_State* L);
  static int lua_receive(lua_State* L);
  static int lua_close(lua_State* L);
  static int lua_unlink(lua_State* L);
  static int lua_clear(lua_State* L);

  static int lua_app_start(lua_State* L);
  static int lua_app_stop(lua_State* L);
  static int lua_app_check_status(lua_State* L);

  static int lua_file_exists(lua_State* L);
  static int lua_file_update(lua_State* L);
  static int lua_file_content(lua_State* L);
  static int lua_file_delete(lua_State* L);
  static int lua_file_backup(lua_State* L);
  static int lua_file_restore(lua_State* L);

  static int lua_folder_exists(lua_State* L);
  static int lua_folder_create(lua_State* L);
  static int lua_folder_delete(lua_State* L);
};
}
