project(luatime)
cmake_minimum_required(VERSION 3.11)

add_library(${PROJECT_NAME} SHARED
    lua_time.cc)

install(TARGETS ${PROJECT_NAME}
    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
    COMPONENT sdl_atf)
