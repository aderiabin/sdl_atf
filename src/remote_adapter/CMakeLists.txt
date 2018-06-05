project(RemoteSDLTestingAdapter)
cmake_minimum_required(VERSION 3.11)
set (CMAKE_CXX_STANDARD 11)
include(FetchContent)

if (UNIX)
  if (QNXNTO)
    set(LINUX FALSE)
  else(QNXNTO)
    set(LINUX TRUE)
  endif(QNXNTO)
endif()

# Enable PIC as static libraries may be linked to shared objects
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)


FetchContent_Declare(
  rpclib
  GIT_REPOSITORY https://github.com/rpclib/rpclib.git
  GIT_TAG        v2.2.1
)

FetchContent_GetProperties(rpclib)
if(NOT rpclib_POPULATED)
  FetchContent_Populate(rpclib)
  add_subdirectory("${rpclib_SOURCE_DIR}" "${rpclib_BINARY_DIR}")
  find_package( Threads REQUIRED )
  add_library(rpc::rpclib ALIAS rpc)
endif()


FIND_PACKAGE(Lua 5.2 EXACT REQUIRED)
message(STATUS "Lua found : " ${LUA_INCLUDE_DIR})
add_subdirectory(remote_adapter_client)
add_subdirectory(remote_adapter_server)
