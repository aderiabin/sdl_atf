#include "lua_lib/sdl_remote_adapter_client_lua_wrapper.h"

#include <iostream>

#include "sdl_remote_adapter_client.h"

namespace lua_lib {

int SDLRemoteClientLuaWrapper::create_SDLRemoteMqClient(lua_State* L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  // Index -1(top) - server port
  // Index -2 - server ip
  // index -3 - Library table

  auto port = lua_tointeger(L, -1);
  auto ip = lua_tostring(L, -2);
  lua_pop(L, 2);  // Remove 2 values from top of the stack
  // Index -1(top) - Library table

  try {
    SDLRemoteTestAdapterClient* client = new SDLRemoteTestAdapterClient(ip, port);

    // Allocate memory for a pointer to client object
    SDLRemoteTestAdapterClient** s =
        (SDLRemoteTestAdapterClient**)lua_newuserdata(
            L, sizeof(SDLRemoteTestAdapterClient*));
    // Index -1(top) - instance userdata
    // Index -2 - Library table

    *s = client;
  } catch (std::exception& e) {
    std::cout << "Exception occurred: " << e.what() << std::endl;
    lua_pushnil(L);
    // Index -1(top) - nil
    // Index -2 - Library table

    return 1;
  }

  SDLRemoteClientLuaWrapper::registerSDLRemoteMqClient(L);
  // Index -1 (top) - registered SDLRemoteMqClient metatable
  // Index -2 - instance userdata
  // Index -3 - Library table

  lua_setmetatable(L, -2);  // Set class table as metatable for instance userdata
  // Index -1(top) - instance userdata
  // Index -2 - Library table

  return 1;
}

int SDLRemoteClientLuaWrapper::destroy_SDLRemoteMqClient(lua_State* L) {
  auto instance = get_instance(L);
  delete instance;
  return 0;
}

void SDLRemoteClientLuaWrapper::registerSDLRemoteMqClient(lua_State* L) {
  static const luaL_Reg SDLRemoteMqClientFunctions[] = {
      {"connected", SDLRemoteClientLuaWrapper::lua_connected},
      {"open", SDLRemoteClientLuaWrapper::lua_open},
      {"open_with_params", SDLRemoteClientLuaWrapper::lua_open_with_params},
      {"send", SDLRemoteClientLuaWrapper::lua_send},
      {"receive", SDLRemoteClientLuaWrapper::lua_receive},
      {"close", SDLRemoteClientLuaWrapper::lua_close},
      {"unlink", SDLRemoteClientLuaWrapper::lua_unlink},
      {"clear", SDLRemoteClientLuaWrapper::lua_clear},
      {"app_start", SDLRemoteClientLuaWrapper::lua_app_start},
      {"app_stop", SDLRemoteClientLuaWrapper::lua_app_stop},
      {"app_check_status", SDLRemoteClientLuaWrapper::lua_app_check_status},
      {"command_execute",SDLRemoteClientLuaWrapper::lua_command_execute},
      {"file_exists", SDLRemoteClientLuaWrapper::lua_file_exists},
      {"file_update", SDLRemoteClientLuaWrapper::lua_file_update},
      {"file_content", SDLRemoteClientLuaWrapper::lua_file_content},
      {"file_delete", SDLRemoteClientLuaWrapper::lua_file_delete},
      {"file_backup", SDLRemoteClientLuaWrapper::lua_file_backup},
      {"file_restore", SDLRemoteClientLuaWrapper::lua_file_restore},
      {"folder_exists", SDLRemoteClientLuaWrapper::lua_folder_exists},
      {"folder_create", SDLRemoteClientLuaWrapper::lua_folder_create},
      {"folder_delete", SDLRemoteClientLuaWrapper::lua_folder_delete},
      {NULL, NULL}
    };

  luaL_newmetatable(L, "SDLRemoteTestAdapterClient");
  // Index -1(top) - SDLRemoteTestAdapter metatable

  lua_newtable(L);
  // Index -1(top) - created table
  // Index -2 : SDLRemoteTestAdapter metatable

  luaL_setfuncs(L, SDLRemoteMqClientFunctions, 0);
  // Index -1(top) - table with SDLRemoteMqClientFunctions
  // Index -2 : SDLRemoteMqClient metatable

  lua_setfield(
      L,
      -2,
      "__index");  // Setup created table as index lookup for  metatable
  // Index -1(top) - SDLRemoteTestAdapter metatable

  lua_pushcfunction(L, SDLRemoteClientLuaWrapper::destroy_SDLRemoteMqClient);
  // Index -1(top) - destroy_SDLRemoteMqClient function pointer
  // Index -2 - SDLRemoteMqClient metatable

  lua_setfield(
      L,
      -2,
      "__gc"); // Set garbage collector function to metatable
  // Index -1(top) - SDLRemoteTestAdapter metatable
}

SDLRemoteTestAdapterClient* SDLRemoteClientLuaWrapper::get_instance(lua_State* L) {
  // Index 1 - userdata instance

  SDLRemoteTestAdapterClient** user_data =
      reinterpret_cast<SDLRemoteTestAdapterClient**>(
          luaL_checkudata(L, 1, "SDLRemoteTestAdapterClient"));

  if (nullptr == user_data) {
    return nullptr;
  }
  return *user_data;  //*((SDLRemoteTestAdapterClient**)ud);
}

int SDLRemoteClientLuaWrapper::lua_connected(lua_State* L) {
  // Index -1(top) - userdata instance

  auto instance = get_instance(L);
  // Index -1(top) - userdata instance

  auto connected = instance->connected();
  lua_pushboolean(L, connected);
  return 1;
}

int SDLRemoteClientLuaWrapper::lua_open(lua_State* L){
  // Index -1(top) - string mq name
  // Index -2 - userdata instance

  auto instance = get_instance(L);
  auto name = lua_tostring(L, -1);
  int result = instance->open(name);
  lua_pushinteger(L, result);
  return 1;
}

int SDLRemoteClientLuaWrapper::lua_open_with_params(lua_State* L) {
  // Index -1(top) - number mode
  // Index -2 - number flags
  // Index -3 - number max_message_size
  // Index -4 - number max_messages_number
  // Index -5 - string name
  // Index -6 - userdata instance

  auto instance = get_instance(L);
  auto name = lua_tostring(L, -5);
  auto max_messages_number = lua_tointeger(L, -4);
  auto max_message_size = lua_tointeger(L, -3);
  auto flags = lua_tointeger(L, -2);
  auto mode = lua_tointeger(L, -1);
  int result = instance->open_with_params(
      name, max_messages_number, max_message_size, flags, mode);
  lua_pushinteger(L, result);
  return 1;
}

int SDLRemoteClientLuaWrapper::lua_send(lua_State* L) {
  // Index -1(top) - string data
  // Index -2 - string mq name
  // Index -3 - userdata instance

  auto instance = get_instance(L);
  auto name = lua_tostring(L, -2);
  auto data = lua_tostring(L, -1);
  int result = instance->send(name, data);
  lua_pushinteger(L, result);
  return 1;
}

int SDLRemoteClientLuaWrapper::lua_receive(lua_State* L) {
  // Index -1(top) - string mq name
  // Index -2 - userdata instance

  auto instance = get_instance(L);
  auto name = lua_tostring(L, -1);
  const auto data_and_error = instance->receive(name);
  lua_pushinteger(L, data_and_error.second);
  lua_pushstring(L, data_and_error.first.c_str());
  return 2;
}

int SDLRemoteClientLuaWrapper::lua_close(lua_State* L) {
  // Index -1(top) - string mq name
  // Index -2 - userdata instance

  auto instance = get_instance(L);
  auto name = lua_tostring(L, -1);
  int result = instance->close(name);
  lua_pushinteger(L, result);
  return 1;
}

int SDLRemoteClientLuaWrapper::lua_unlink(lua_State* L) {
  // Index -1(top) - string mq name
  // Index -2 - userdata instance

  auto instance = get_instance(L);
  auto name = lua_tostring(L, -1);
  int result = instance->unlink(name);
  lua_pushinteger(L, result);
  return 1;
}

int SDLRemoteClientLuaWrapper::lua_clear(lua_State* L) {
  // Index -1(top) - userdata instance

  auto instance = get_instance(L);
  int result = instance->clear();
  lua_pushinteger(L, result);
  return 1;
}

int SDLRemoteClientLuaWrapper::lua_app_start(lua_State* L) {
  // Index -1(top) - string app name
  // Index -2 - string path to app
  // Index -3 - userdata instance

  auto instance = get_instance(L);
  auto path = lua_tostring(L, -2);
  auto name = lua_tostring(L, -1);
  int result = instance->app_start(path, name);
  lua_pushinteger(L, result);
  return 1;
}

int SDLRemoteClientLuaWrapper::lua_app_stop(lua_State* L) {
  // Index -1(top) - string app name
  // Index -2 - userdata instance

  auto instance = get_instance(L);
  auto name = lua_tostring(L, -1);
  int result = instance->app_stop(name);
  lua_pushinteger(L, result);
  return 1;
}

int SDLRemoteClientLuaWrapper::lua_app_check_status(lua_State* L) {
  // Index -1(top) - string app name
  // Index -2 - userdata instance

  auto instance = get_instance(L);
  auto name = lua_tostring(L, -1);
  const auto data_and_error = instance->app_check_status(name);
  lua_pushinteger(L, data_and_error.second);
  lua_pushinteger(L, data_and_error.first);
  return 2;
}

int SDLRemoteClientLuaWrapper::lua_file_exists(lua_State* L) {
  // Index -1(top) - string file name
  // Index -2 - string path to file
  // Index -3 - userdata instance

  auto instance = get_instance(L);
  auto path = lua_tostring(L, -2);
  auto name = lua_tostring(L, -1);
  const auto data_and_error = instance->file_exists(path, name);
  lua_pushinteger(L, data_and_error.second);
  lua_pushboolean(L, data_and_error.first);
  return 2;
}

int SDLRemoteClientLuaWrapper::lua_file_update(lua_State* L) {
  // Index -1(top) - string file content
  // Index -2 - string file name
  // Index -3 - string path to file
  // Index -4 - userdata instance

  auto instance = get_instance(L);
  auto path = lua_tostring(L, -3);
  auto name = lua_tostring(L, -2);
  auto content = lua_tostring(L, -1);
  int result = instance->file_update(path, name, content);
  lua_pushinteger(L, result);
  return 1;
}

int SDLRemoteClientLuaWrapper::lua_file_content(lua_State* L) {
  // Index -1(top) - string file name
  // Index -2 - string path to file
  // Index -3 - userdata instance

  auto instance = get_instance(L);
  auto path = lua_tostring(L, -2);
  auto name = lua_tostring(L, -1);
  const auto data_and_error = instance->file_content(path, name);
  lua_pushinteger(L, data_and_error.second);
  lua_pushstring(L, data_and_error.first.c_str());
  return 2;
}

int SDLRemoteClientLuaWrapper::lua_file_delete(lua_State* L) {
  // Index -1(top) - string file name
  // Index -2 - string path to file
  // Index -3 - userdata instance

  auto instance = get_instance(L);
  auto path = lua_tostring(L, -2);
  auto name = lua_tostring(L, -1);
  int result = instance->file_delete(path, name);
  lua_pushinteger(L, result);
  return 1;
}

int SDLRemoteClientLuaWrapper::lua_file_backup(lua_State* L) {
  // Index -1(top) - string file name
  // Index -2 - string path to file
  // Index -3 - userdata instance

  auto instance = get_instance(L);
  auto path = lua_tostring(L, -2);
  auto name = lua_tostring(L, -1);
  int result = instance->file_backup(path, name);
  lua_pushinteger(L, result);
  return 1;
}

int SDLRemoteClientLuaWrapper::lua_file_restore(lua_State* L) {
  // Index -1(top) - string file name
  // Index -2 - string path to file
  // Index -3 - userdata instance

  auto instance = get_instance(L);
  auto path = lua_tostring(L, -2);
  auto name = lua_tostring(L, -1);
  int result = instance->file_restore(path, name);
  lua_pushinteger(L, result);
  return 1;
}

int SDLRemoteClientLuaWrapper::lua_folder_exists(lua_State* L) {
  // Index -1(top) - string path to folder
  // Index -2 - userdata instance

  auto instance = get_instance(L);
  auto path = lua_tostring(L, -1);
  const auto data_and_error = instance->folder_exists(path);
  lua_pushinteger(L, data_and_error.second);
  lua_pushboolean(L, data_and_error.first);
  return 2;
}

int SDLRemoteClientLuaWrapper::lua_folder_create(lua_State* L) {
  // Index -1(top) - string folder path
  // Index -2 - userdata instance

  auto instance = get_instance(L);
  auto path = lua_tostring(L, -1);
  int result = instance->folder_create(path);
  lua_pushinteger(L, result);
  return 1;
}

int SDLRemoteClientLuaWrapper::lua_folder_delete(lua_State* L) {
  // Index -1(top) - string path to folder
  // Index -2 - userdata instance

  auto instance = get_instance(L);
  auto path = lua_tostring(L, -1);
  int result = instance->folder_delete(path);
  lua_pushinteger(L, result);
  return 1;
}

int SDLRemoteClientLuaWrapper::lua_command_execute(lua_State* L){
  // Index -1(top) - bash_command
  // Index -2 - userdata instance

  auto instance = get_instance(L);
  auto bash_command = lua_tostring(L, -1);
  const auto data_and_error = instance->command_execute(bash_command);
  lua_pushinteger(L, data_and_error.second);
  lua_pushstring(L, data_and_error.first.c_str());
  return 2;
}

}  // namespace lua_lib
