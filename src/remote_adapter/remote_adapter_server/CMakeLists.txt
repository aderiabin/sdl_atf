project(RemoteTestingAdapterServer)
cmake_minimum_required(VERSION 3.11)
set (CMAKE_CXX_STANDARD 11)
include(FetchContent)

# Enable PIC as static libraries may be linked to shared objects
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

if (UNIX)
  if (QNXNTO)
    set(LINUX FALSE)
  else(QNXNTO)
    set(LINUX TRUE)
  endif(QNXNTO)
endif()

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

find_package( Threads REQUIRED )
add_executable(${PROJECT_NAME}
                main.cc
                mqueue_manager.cc
                shmemory_manager.cc
                utils_manager.cc)

target_link_libraries(${PROJECT_NAME}
                       rpc::rpclib
                       Threads::Threads
                       $<$<BOOL:${LINUX}>:rt>
                       $<$<BOOL:${QNXNTO}>:mq>
                       $<$<BOOL:${QNXNTO}>:socket>)

target_include_directories( ${PROJECT_NAME}  PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/..
)

install(
  TARGETS ${PROJECT_NAME}
  DESTINATION "${CMAKE_INSTALL_PREFIX}/RemoteTestingAdapterServer"
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
  COMPONENT sdl_atf
)
