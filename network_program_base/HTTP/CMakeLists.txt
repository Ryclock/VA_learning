cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME HTTP)
project(${PROJECT_NAME})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_FLAG -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/build/bin/debug)    
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/build/bin/release)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/build/lib/debug)    
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/build/lib/release)

include_directories(include)

set(SERVER_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/server/server.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/server/main.cpp
)
add_executable(server ${SERVER_SOURCES})

set(CLIENT_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/client/request.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/client/main.cpp
)
add_executable(client ${CLIENT_SOURCES})

file(GLOB_RECURSE UTILS_LIB_SRC
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/*.cpp
)
add_library(utils STATIC ${UTILS_LIB_SRC})
target_link_libraries(server PRIVATE utils)
target_link_libraries(client PRIVATE utils)


set(quirc_DIR ${CMAKE_CURRENT_LIST_DIR}/packages/share/quirc)
set(TIFF_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/packages/lib/libtiff.a)
set(TIFF_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/packages/include)
set(Protobuf_DIR ${CMAKE_CURRENT_LIST_DIR}/packages/share/protobuf)
set(OpenCV_DIR ${CMAKE_CURRENT_LIST_DIR}/packages/share/opencv4)
find_package(OpenCV CONFIG REQUIRED)
target_link_libraries(server PRIVATE opencv_core opencv_imgcodecs)
target_link_libraries(client PRIVATE opencv_core opencv_imgcodecs)

set(Libevent_DIR ${CMAKE_CURRENT_LIST_DIR}/packages/share/libevent)
find_package(Libevent CONFIG REQUIRED)
target_link_libraries(server PRIVATE libevent::core libevent::extra)

set(jsoncpp_DIR ${CMAKE_CURRENT_LIST_DIR}/packages/share/jsoncpp)
find_package(jsoncpp CONFIG REQUIRED)
target_link_libraries(server PRIVATE JsonCpp::JsonCpp)
target_link_libraries(client PRIVATE JsonCpp::JsonCpp)

set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/packages/include)
set(ZLIB_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/packages/lib/libzlib.a)
set(CURL_DIR ${CMAKE_CURRENT_LIST_DIR}/packages/share/curl)
find_package(CURL CONFIG REQUIRED)
target_link_libraries(client PRIVATE CURL::libcurl)
