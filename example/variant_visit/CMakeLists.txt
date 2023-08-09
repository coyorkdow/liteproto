cmake_minimum_required(VERSION 3.13)
project(variant_visit)


add_compile_options(-Wall)

add_executable(variant_visit variant_visit.cpp)
target_link_libraries(variant_visit liteproto)