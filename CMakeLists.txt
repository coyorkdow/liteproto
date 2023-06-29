cmake_minimum_required(VERSION 3.13)
project(liteproto)

set(CMAKE_CXX_STANDARD 17)

list(APPEND liteproto_headers
        include/liteproto/liteproto.hpp
        include/liteproto/message.hpp
        include/liteproto/utils.hpp
        include/liteproto/reflect/type.hpp
        include/liteproto/interface.hpp
        include/liteproto/list.hpp
        include/liteproto/reflect/object.hpp
        include/liteproto/static_test/static_test.hpp)

add_compile_options(-g -Wall)

add_library(liteproto STATIC src/liteproto.cpp)

target_include_directories(liteproto PUBLIC include)
#target_compile_definitions(liteproto PUBLIC LITE_PROTO_DISABLE_COMPATIBLE_MODE_)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(install_gtest OFF)
set(install_gmock OFF)
set(build_gmock ON)
add_subdirectory(./googletest)

add_executable(liteproto_test test/test.cpp)
target_link_libraries(liteproto_test liteproto gmock gtest gtest_main)

add_test(NAME closure_test COMMAND closure_test)