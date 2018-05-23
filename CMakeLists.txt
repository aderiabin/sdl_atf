project(ATF)
cmake_minimum_required(VERSION 3.1)
include(FetchContent)

# Enable PIC as static libraries may be linked to shared objects
set (CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Network WebSockets REQUIRED)


FIND_PACKAGE(Lua 5.2 EXACT REQUIRED)

list(GET LUA_LIBRARIES 0 LUA_LIB)
list(GET LUA_LIBRARIES 1 LIBM_LIB)

message(STATUS "lualib : ${LUA_LIB}")
message(STATUS "libmlib : ${LIBM_LIB}")

add_library(lua::lua UNKNOWN IMPORTED GLOBAL)
set_target_properties(lua::lua
  PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
    IMPORTED_LOCATION ${LUA_LIB}
    IMPORTED_LINK_DEPENDENT_LIBRARIES ${LIBM_LIB}
    )


FetchContent_Declare(
  MqTCP
  GIT_REPOSITORY git@github.ford.com:a-kutsan/MqTCP.git
  GIT_TAG        master
)

FetchContent_GetProperties(MqTCP)
if(NOT MqTCP_POPULATED)
  FetchContent_Populate(MqTCP)
  add_subdirectory("${mqtcp_SOURCE_DIR}" "${mqtcp_BINARY_DIR}")
endif() 

add_executable(${PROJECT_NAME}
                src/network.cc 
                src/timers.cc 
                src/qtdynamic.cc 
                src/qtlua.cc 
                src/qdatetime.cc 
                src/marshal.cc 
                src/main.cc 
                src/lua_interpreter.cc
                )

target_link_libraries(${PROJECT_NAME}
                       Qt5::Core
                       Qt5::Network
                       Qt5::WebSockets
                       lua::lua
                       )