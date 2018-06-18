#include "qt_impl/lua_lib/sdl_remote_adapter_qt_client_lua_wrapper.h"

#include <iostream>

#include "qt_impl/sdl_remote_adapter_qt_client.h"

namespace lua_lib {

int SDLRemoteTestAdapterLuaWrapper::create_SDLRemoteTestAdapter(lua_State* L) {
  luaL_checktype(L, 1, LUA_TTABLE);

  // Index -1(top) - table out_params
  // Index -2 - table in_params
  // Index -3 - number port
  // Index -4 - string host
  // index -5 - Library table

  auto out_mq_params = build_MqParams(L);
  lua_pop(L, 1);  // Remove value from the top of the stack
  // Index -1(top) - table in_params
  // Index -2 - number port
  // Index -3 - string host
  // index -4 - Library table

  auto in_mq_params = build_MqParams(L);
  lua_pop(L, 1);  // Remove value from the top of the stack
  // Index -1(top) - number port
  // Index -2 - string host
  // Index -3 - Library table

  auto port = lua_tointeger(L, -3);
  auto ip = lua_tostring(L, -4);
  lua_pop(L, 2);  // Remove 2 values from top of the stack
  // Index -1(top) - Library table

  lua_newtable(L);  // Create instance table
  // Index -1(top) - created instance table
  // Index -2 - Library table

  SDLRemoteTestAdapterLuaWrapper::registerSDLRemoteTestAdapter(L);
  // Index -1 (top) - registered class table
  // Index -2 - created instance table
  // Index -3 - Library table

  lua_setmetatable(L, -2);  // Set class table as metatable for instance table
  // Index -1(top) - created instance table

  // Allocate memory for a pointer to client object
  SDLRemoteTestAdapterQtClient** s =
      (SDLRemoteTestAdapterQtClient**)lua_newuserdata(
          L, sizeof(SDLRemoteTestAdapterQtClient*));
  // Index -1(top) - new userdata
  // Index -2 - created instance table

  try {
    *s = new SDLRemoteTestAdapterQtClient(ip, port, in_mq_params, out_mq_params);
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
      {"connect", SDLRemoteTestAdapterLuaWrapper::lua_connect},
      {"write", SDLRemoteTestAdapterLuaWrapper::lua_write},

      {NULL, NULL}};

  lua_newtable(L);
  luaL_setfuncs(L, SDLRemoteTestAdapterFunctions, 0);
  // Index -1(top) : SDLRemoteTestAdapter class table

//   lua_pushvalue(L, -1);
//   // Index -1(top) : SDLRemoteTestAdapter class table
//   // Index -2 : SDLRemoteTestAdapter class table

//   lua_setglobal(L, "SDLRemoteTestAdapter");
//   // Index -1(top) : SDLRemoteTestAdapter class table

  lua_pushvalue(L, -1);
  // Index -1(top) : SDLRemoteTestAdapter class table
  // Index -2 : SDLRemoteTestAdapter class table

  lua_setfield(
      L,
      -2,
      "__index");  // Setup index lookup to it self for using it as metatable
  // Index -1(top) : SDLRemoteTestAdapter class table

  lua_pushcfunction(L, SDLRemoteTestAdapterLuaWrapper::destroy_SDLRemoteTestAdapter);
  // Index -1(top) : destroy_SDLRemoteTestAdapter function pointer
  // Index -2 : SDLRemoteTestAdapter class table

  lua_setfield(
      L,
      -2,
      "__gc"); // Set garbage collector function to metatable
  // Index -1(top) : SDLRemoteTestAdapter class table
}

SDLRemoteTestAdapterQtClient* SDLRemoteTestAdapterLuaWrapper::get_instance(
    lua_State* L) {
  // Index 1 - lua instance
  lua_getfield(
      L, 1, "__self");  // get __self key from lua instance and put on top
  // Index -1 (top) - user data ( poiner to C++ implementation pointer)
  SDLRemoteTestAdapterQtClient** user_data =
      reinterpret_cast<SDLRemoteTestAdapterQtClient**>(lua_touserdata(L, -1));
  lua_pop(L, 1);  // Remove user data from the top
  if (nullptr == user_data) {
    return nullptr;
  }
  SDLRemoteTestAdapterQtClient* adapter = *user_data;
  return adapter;  //*((SDLRemoteTestAdapterQtClient**)ud);
}

MqParams SDLRemoteTestAdapterLuaWrapper::build_MqParams(lua_State* L) {
  // Index -1(top) - table params

  lua_getfield(
      L, -1, "name");  //Pushes onto the stack the value params[name]
  // Index -1(top) - string name
  // Index -2 - table params

  const char* name = lua_tostring(L, -1);
  lua_pop(L, 1);  // remove value from the top of the stack
  // Index -1(top) - table params

  lua_getfield(
      L, -1, "max_messages_number");
      //Pushes onto the stack the value params[max_messages_number]
  // Index -1(top) - number max_messages_number
  // Index -2 - table params

  const int max_messages_number = lua_tointeger(L, -1);
  lua_pop(L, 1);  // Remove value from the top of the stack
  // Index -1(top) - table params

  lua_getfield(
      L, -1, "max_message_size");
      //Pushes onto the stack the value params[max_message_size]
  // Index -1(top) - number max_message_size
  // Index -2 - table params

  const int max_message_size = lua_tointeger(L, -1);
  lua_pop(L, 1);  // Remove value from the top of the stack
  // Index -1(top) - table params

  lua_getfield(
      L, -1, "flags");  //Pushes onto the stack the value params[flags]
  // Index -1(top) - number flags
  // Index -2 - table params

  const int flags = lua_tointeger(L, -1);
  lua_pop(L, 1);  // Remove value from the top of the stack
  // Index -1(top) - table params

  lua_getfield(
      L, -1, "mode");  //Pushes onto the stack the value params[mode]
  // Index -1(top) - number mode
  // Index -2 - table params

  const int mode = lua_tointeger(L, -1);
  lua_pop(L, 1);  // Remove value from the top of the stack
  // Index -1(top) - table params

  MqParams mq_params = {
      std::string(name),
      max_messages_number,
      max_message_size,
      flags,
      mode
  };

  return mq_params;
}

int SDLRemoteTestAdapterLuaWrapper::lua_connect(lua_State* L) {
  // Index -1(top) - table instance

  auto instance = get_instance(L);
  // Index -1(top) - table instance

  instance->connectMq();
  return 0;
}

int SDLRemoteTestAdapterLuaWrapper::lua_write(lua_State* L) {
  // Index -1(top) - string data
  // Index -2 - table instance

  auto instance = get_instance(L);
  auto data = lua_tostring(L, -1);
  int result = instance->send(data);
  lua_pushinteger(L, result);
  return 1;
}

}  // namespace lua_lib
