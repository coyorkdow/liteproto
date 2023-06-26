cmake_minimum_required(VERSION 3.24)
project(liteproto)

set(CMAKE_CXX_STANDARD 17)

list(APPEND liteproto_headers
        include/liteproto/message.hpp
        include/liteproto/utils.hpp
        include/liteproto/type.hpp
        include/liteproto/interface.hpp
        include/liteproto/list.hpp
        include/liteproto/reflection/object.hpp)

add_executable(liteproto main.cpp)

target_include_directories(liteproto PUBLIC include)
target_compile_definitions(liteproto PUBLIC LITE_PROTO_COMPATIBLE_MODE_)