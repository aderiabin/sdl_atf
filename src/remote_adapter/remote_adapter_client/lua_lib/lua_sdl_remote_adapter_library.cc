#include "lua_lib/sdl_remote_adapter_client_lua_wrapper.h"

extern "C" {

int luaopen_libRemoteTestingAdapterClient (lua_State *L){

    static const luaL_Reg library_functions[] = {
        {"new", lua_lib::SDLRemoteTestAdapterLuaWrapper::create_SDLRemoteTestAdapter},
        {NULL, NULL}
    };

    luaL_newlib(L, library_functions);
    return 1;
}
}
