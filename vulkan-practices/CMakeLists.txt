
project (vulkan-practices)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# GLFW
set (GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("dep/glfw")

# glm
include_directories ("dep/glm")

# project structure
set (target vulkan01)
set (sources Main.cpp)

add_executable (${target} ${sources})
target_link_libraries (${target} glfw vulkan)
