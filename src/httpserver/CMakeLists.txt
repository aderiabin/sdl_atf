cmake_minimum_required(VERSION 3.1)

project(httpserver)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Network REQUIRED)

add_library(${PROJECT_NAME} STATIC
                                "httpglobal.cpp"
                                "httplistener.cpp"
                                "httpconnectionhandler.cpp"
                                "httpconnectionhandlerpool.cpp"
                                "httprequest.cpp"
                                "httpresponse.cpp"
                                "httpcookie.cpp"
                                "httprequesthandler.cpp"
                                "httpsession.cpp"
                                "httpsessionstore.cpp"
                                "staticfilecontroller.cpp"
                                )

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Network)
