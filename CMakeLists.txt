project(atf_web_app)
cmake_minimum_required(VERSION 3.1)

# Enable PIC as static libraries may be linked to shared objects
set (CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)
# Append directories in the linker search path and outside the project
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

find_package(Qt5 COMPONENTS Core Network REQUIRED)
find_package(Lua 5.2 EXACT REQUIRED)

list(GET LUA_LIBRARIES 0 LUA_LIB)
list(GET LUA_LIBRARIES 1 LIBM_LIB)

add_library(lua::lua UNKNOWN IMPORTED GLOBAL)
set_target_properties(lua::lua
  PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
    IMPORTED_LOCATION ${LUA_LIB}
    IMPORTED_LINK_DEPENDENT_LIBRARIES ${LIBM_LIB}
    )

execute_process(COMMAND git submodule init WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
execute_process(COMMAND git submodule update WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

include_directories("src"
                    "src/controller"
                    "src/sdl"
                    "src/httpserver"
                    "src/logging"
                    "src/templateengine"
                    )

add_subdirectory("src/logging" "logging")
add_subdirectory("src/httpserver" "httpserver")
add_subdirectory("src/templateengine" "templateengine")

add_executable(${PROJECT_NAME}  "src/main.cpp"
                                "src/global.cpp"
                                "src/requestmapper.cpp"
                                "src/controller/dumpcontroller.cpp"
                                "src/controller/templatecontroller.cpp"
                                "src/controller/formcontroller.cpp"
                                "src/controller/fileuploadcontroller.cpp"
                                "src/controller/sessioncontroller.cpp"
                                "src/controller/sdlcontrolcontroller.cpp"
                                "src/controller/sdldatacontroller.cpp"
                                "src/sdl/network.cc"
                                "src/sdl/timers.cc"
                                "src/sdl/qtdynamic.cc"
                                "src/sdl/qtlua.cc"
                                "src/sdl/qdatetime.cc"
                                "src/sdl/marshal.cc"
                                "src/sdl/lua_interpreter.cc"
                                )

target_link_libraries(${PROJECT_NAME}   Qt5::Core
                                        Qt5::Network
                                        lua::lua
                                        httpserver
                                        logging
                                        templateengine
                                        )

include("BSON.cmake")

install (
    TARGETS ${PROJECT_NAME}
    DESTINATION "${CMAKE_INSTALL_PREFIX}"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
)

install(
  DIRECTORY
  "${CMAKE_SOURCE_DIR}/etc"
  "${CMAKE_SOURCE_DIR}/logs"
  "${CMAKE_SOURCE_DIR}/modules"
  "${CMAKE_SOURCE_DIR}/data"
  DESTINATION "${CMAKE_INSTALL_PREFIX}"
  FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
)

install(
  DIRECTORY
  "${CMAKE_SOURCE_DIR}/tools"
  DESTINATION "${CMAKE_INSTALL_PREFIX}"
  FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
  COMPONENT sdl_atf
)

# install(
#   FILES
#   "${CMAKE_SOURCE_DIR}/../readme.txt"
#   DESTINATION "${CMAKE_INSTALL_PREFIX}"
#   PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
# )

install(CODE "execute_process( \
    COMMAND ${CMAKE_COMMAND} -E create_symlink \
    ./libluabson.so \
    ${CMAKE_INSTALL_PREFIX}/lib/libbson4lua.so \
    )"
)

add_subdirectory(src/sdl/luaxml)
add_subdirectory(src/sdl/luaopenssl)
