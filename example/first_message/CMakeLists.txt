cmake_minimum_required(VERSION 3.13)
project(first_message)


add_compile_options(-Wall)

add_executable(first_message first_message.cpp)
target_link_libraries(first_message liteproto)