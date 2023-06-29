cmake_minimum_required(VERSION 3.24)
project(liteproto)

set(CMAKE_CXX_STANDARD 20)

list(APPEND liteproto_headers
        include/liteproto/liteproto.hpp
        include/liteproto/message.hpp
        include/liteproto/utils.hpp
        include/liteproto/reflect/type.hpp
        include/liteproto/interface.hpp
        include/liteproto/list.hpp
        include/liteproto/reflect/object.hpp)

add_compile_options(-g)

add_executable(liteproto main.cpp)

target_include_directories(liteproto PUBLIC include)
#target_compile_definitions(liteproto PUBLIC LITE_PROTO_DISABLE_COMPATIBLE_MODE_)