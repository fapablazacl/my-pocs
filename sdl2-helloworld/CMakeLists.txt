cmake_minimum_required(VERSION 3.10)

project("sdl2-helloworld")

find_package(SDL2 REQUIRED)

set (target "sdl2-helloworld")
set (sources "main.cpp")

add_executable(${target} ${sources})
target_link_libraries(${target} PRIVATE SDL2::SDL2)
