
cmake_minimum_required(VERSION 3.15)

project (testOpenSceneGraph)


find_package (OpenSceneGraph REQUIRED osgDB osgUtil osgViewer osgGA)

add_executable(testOpenSceneGraph main.cpp)

target_include_directories(testOpenSceneGraph PRIVATE ${OPENSCENEGRAPH_INCLUDE_DIRS})
target_link_libraries(testOpenSceneGraph ${OPENSCENEGRAPH_LIBRARIES})

