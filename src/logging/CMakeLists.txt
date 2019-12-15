cmake_minimum_required(VERSION 3.1)

project(logging)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Network REQUIRED)

add_library(${PROJECT_NAME} STATIC
                                "logmessage.cpp"
                                "logger.cpp"
                                "filelogger.cpp"
                                "dualfilelogger.cpp"
                                )

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Network)
