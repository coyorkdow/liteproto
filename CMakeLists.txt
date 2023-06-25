cmake_minimum_required(VERSION 3.24)
project(liteproto)

set(CMAKE_CXX_STANDARD 17)

list(APPEND liteproto_headers
        include/liteproto/message.hpp
        include/liteproto/reflection.hpp
        include/liteproto/utils.hpp
        include/liteproto/type.hpp
        include/liteproto/interface.hpp)

add_executable(liteproto main.cpp)

target_include_directories(liteproto PUBLIC include)