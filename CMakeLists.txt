cmake_minimum_required(VERSION 3.5)

# set the project name
project(GameServer)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PROTOBUF_INCLUDE_DIRS "/usr/local/protobuf/include")
set(PROTOBUF_LIBRARY_DIRS "/usr/local/protobuf/lib")
find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif()

include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/protobuf)

link_directories(${PROTOBUF_LIBRARY_DIRS})

# # add the executable

add_executable(GameServer game_server.cpp)

target_link_libraries(GameServer PUBLIC muduo_net game_core codec)

add_subdirectory(protobuf)
add_subdirectory(muduo/base)
add_subdirectory(muduo/net)
add_subdirectory(game_core)


