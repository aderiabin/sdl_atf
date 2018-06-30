project(RemoteHMIAdapter)
cmake_minimum_required(VERSION 3.11)
include(FetchContent)

# Enable PIC as static libraries may be linked to shared objects
set (CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)

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

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package( Threads REQUIRED )
find_package(Lua 5.2 EXACT REQUIRED)

list(GET LUA_LIBRARIES 0 LUA_LIB)
list(GET LUA_LIBRARIES 1 LIBM_LIB)
message(STATUS "lualib : ${LUA_LIB}")
message(STATUS "libmlib : ${LIBM_LIB}")

if (NOT TARGET lua::lua)
  add_library(lua::lua UNKNOWN IMPORTED GLOBAL)

  set_target_properties(lua::lua
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
      IMPORTED_LOCATION ${LUA_LIB}
      IMPORTED_LINK_DEPENDENT_LIBRARIES ${LIBM_LIB}
      )
endif()

add_library(${PROJECT_NAME} SHARED
                 sdl_remote_adapter_client.cc
                 qt_impl/sdl_remote_adapter_receive_thread.cc
                 qt_impl/sdl_remote_adapter_qt_client.cc
                 qt_impl/lua_lib/sdl_remote_adapter_qt_client_lua_wrapper.cc
                 qt_impl/lua_lib/lua_sdl_remote_adapter_library.cc
)

target_include_directories(${PROJECT_NAME} PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/..
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${LUA_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME}
                        Qt5::Core
                        lua::lua
                        rpc::rpclib
                        Threads::Threads
                        $<$<BOOL:${QNXNTO}>:socket>)

install(
  TARGETS ${PROJECT_NAME}
  DESTINATION "${CMAKE_INSTALL_PREFIX}"
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
  COMPONENT sdl_atf
)
