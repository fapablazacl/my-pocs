cmake_minimum_required(VERSION 3.10)

project (3dgraphics)

find_package(assimp REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_subdirectory(glad)

add_executable(3dgraphics 3dgraphics.cpp)
target_link_libraries(3dgraphics assimp glfw GL glm glad)
