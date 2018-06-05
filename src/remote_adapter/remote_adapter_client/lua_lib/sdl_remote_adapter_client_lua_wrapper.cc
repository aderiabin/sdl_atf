#include "lua_lib/sdl_remote_adapter_client_lua_wrapper.h"

#include <iostream>

#include "sdl_remote_adapter_client.h"

namespace lua_lib {

int SDLRemoteTestAdapterLuaWrapper::create_SDLRemoteTestAdapter(lua_State* L) {
  luaL_checktype(L, 1, LUA_TTABLE);

  auto ip = lua_tostring(L, -2);
  auto port = lua_tointeger(L, -1);
  lua_pop(L, 2);  // Remove from top of stack 2 variables
  // Index -1(top) - new created table (instance)

  lua_newtable(L);  // Create instance table
  // Index -2 - Library table
  // Index -1(top) - new created table (instance)

  SDLRemoteTestAdapterLuaWrapper::registerSDLRemoteTestAdapter(L);
  // Index -1 (top) - registered class table
  // Index -2 - created instance table
  // Index -3 - Library table

  lua_setmetatable(L, -2);  // Set class table as metatable for instance table
  // Index -1(top) - created instance table

  // Allocate memory for a pointer to to object
  SDLRemoteTestAdapterClient** s =
      (SDLRemoteTestAdapterClient**)lua_newuserdata(
          L, sizeof(SDLRemoteTestAdapterClient*));
  // Index -1(top) - new user data
  // Index -2 - created instance table

  try {
    *s = new SDLRemoteTestAdapterClient(ip, port);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    return 0;
  }
  lua_setfield(
      L,
      -2,
      "__self");  // Set field '__self' of instance table to the user data
  // Index -1(top) - created instance table
  return 1;
}

int SDLRemoteTestAdapterLuaWrapper::destroy_SDLRemoteTestAdapter(lua_State* L) {
  auto instance = get_instance(L);
  delete instance;
  return 0;
}

void SDLRemoteTestAdapterLuaWrapper::registerSDLRemoteTestAdapter(
    lua_State* L) {
  static const luaL_Reg SDLRemoteTestAdapterFunctions[] = {
      {"__gc", SDLRemoteTestAdapterLuaWrapper::destroy_SDLRemoteTestAdapter},

      {"connected", SDLRemoteTestAdapterLuaWrapper::lua_connected},
      {"open", SDLRemoteTestAdapterLuaWrapper::lua_open},
      {"send", SDLRemoteTestAdapterLuaWrapper::lua_send},
      {"receive", SDLRemoteTestAdapterLuaWrapper::lua_receive},
      {"clear", SDLRemoteTestAdapterLuaWrapper::lua_clear},

      {NULL, NULL}};

  lua_newtable(L);
  luaL_setfuncs(L, SDLRemoteTestAdapterFunctions, 0);
  // Index -1(top) : SDLRemoteTestAdapter class table

  lua_pushvalue(L, -1);
  // Index -1(top) : SDLRemoteTestAdapter class table
  // Index -2 : SDLRemoteTestAdapter class table

  lua_setglobal(L, "SDLRemoteTestAdapter");
  // Index -1(top) : SDLRemoteTestAdapter class table

  lua_pushvalue(L, -1);
  // Index -1(top) : SDLRemoteTestAdapter class table
  // Index -2 : SDLRemoteTestAdapter class table

  lua_setfield(
      L,
      -2,
      "__index");  // Setup index lookup to it self for using it as metatable
                   // Index -1(top) : SDLRemoteTestAdapter class table
}

SDLRemoteTestAdapterClient* SDLRemoteTestAdapterLuaWrapper::get_instance(
    lua_State* L) {
  // Index 1 - lua instance
  lua_getfield(
      L, 1, "__self");  // get __self key from lua instance and put on top
  // Index -1 (top) - user data ( poiner to C++ implementation pointer)
  SDLRemoteTestAdapterClient** user_data =
      reinterpret_cast<SDLRemoteTestAdapterClient**>(lua_touserdata(L, -1));
  lua_pop(L, 1);  // Remove user data from the top
  if (nullptr == user_data) {
    return nullptr;
  }
  SDLRemoteTestAdapterClient* adapter = *user_data;
  return adapter;  //*((SDLRemoteTestAdapter**)ud);
}

int SDLRemoteTestAdapterLuaWrapper::lua_connected(lua_State* L) {
  auto instance = get_instance(L);
  auto connected = instance->connected();
  lua_pushboolean(L, connected);
  return 1;
}

int SDLRemoteTestAdapterLuaWrapper::lua_open(lua_State* L) {
  auto instance = get_instance(L);
  auto name = lua_tostring(L, -1);
  int success = instance->open(name);
  lua_pushinteger(L, success);
  return 1;
}

int SDLRemoteTestAdapterLuaWrapper::lua_open_with_params(lua_State* L) {
  auto instance = get_instance(L);
  auto name = lua_tostring(L, -5);
  auto max_messages_number = lua_tointeger(L, -4);
  auto max_message_size = lua_tointeger(L, -3);
  auto flags = lua_tointeger(L, -2);
  auto mode = lua_tointeger(L, -1);
  int success = instance->open_with_params(
      name, max_messages_number, max_message_size, flags, mode);
  lua_pushinteger(L, success);
  return 1;
}

int SDLRemoteTestAdapterLuaWrapper::lua_send(lua_State* L) {
  auto instance = get_instance(L);
  auto name = lua_tostring(L, -2);
  auto data = lua_tostring(L, -1);
  int success = instance->send(name, data);
  lua_pushinteger(L, success);
  return 1;
}

int SDLRemoteTestAdapterLuaWrapper::lua_receive(lua_State* L) {
  auto instance = get_instance(L);
  auto name = lua_tostring(L, -1);
  const auto data_and_error = instance->receive(name);
  lua_pushinteger(L, data_and_error.second);
  lua_pushstring(L, data_and_error.first.c_str());
  return 2;
}

int SDLRemoteTestAdapterLuaWrapper::lua_clear(lua_State* L) {
  auto instance = get_instance(L);
  int success = instance->clear();
  lua_pushinteger(L, success);
  return 1;
}
}  // namespace lua_lib
