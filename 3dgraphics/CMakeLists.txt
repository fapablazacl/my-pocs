cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project (3dgraphics)

find_package(assimp REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_subdirectory(glad)

add_executable(3dgraphics 3dgraphics.cpp)
target_link_libraries(3dgraphics assimp::assimp glfw glm::glm glad)
