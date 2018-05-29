project(RemoteTestingAdapterServer)
cmake_minimum_required(VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)

find_package( Threads REQUIRED )
add_executable(${PROJECT_NAME}
                 main.cc
                 mqueue_manager.cc)

target_link_libraries(${PROJECT_NAME}
                       rpc::rpclib
                       Threads::Threads
                       $<$<BOOL:${LINUX}>:rt>
                       $<$<BOOL:${QNXNTO}>:mq>
                       $<$<BOOL:${QNXNTO}>:socket>)

target_include_directories( ${PROJECT_NAME}  PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/..
)
