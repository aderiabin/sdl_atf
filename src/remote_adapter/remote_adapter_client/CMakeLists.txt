project(RemoteTestingAdapterClient)
cmake_minimum_required(VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)

find_package( Threads REQUIRED )
FIND_PACKAGE(Lua 5.2 EXACT REQUIRED)
message(STATUS "Lua found : " ${LUA_INCLUDE_DIR})

add_library(${PROJECT_NAME} SHARED
                 sdl_remote_adapter_client.cc
                 lua_lib/lua_sdl_remote_adapter_library.cc
                 lua_lib/sdl_remote_adapter_client_lua_wrapper.cc
)

target_include_directories(${PROJECT_NAME} PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/..
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${LUA_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME}
                        rpc::rpclib
                        Threads::Threads
                        $<$<BOOL:${QNXNTO}>:socket>)
