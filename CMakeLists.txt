cmake_minimum_required(VERSION 3.15.0)
project(gltf_renderer)

# set standerd
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


include_directories(external/glfw/include)
find_library(GLFW_LIBRARY glfw3 PATHS ${CMAKE_SOURCE_DIR}/external/glfw/lib-vc2022)
# set(GLFW_LIBRARY external/glfw/lib-vc2022/glfw3)  # not sure why it doesnt it this way?

# add glad
include_directories(external/glad/include)
add_library(glad STATIC external/glad/src/glad.c)

#find_package(glfw3 3.3 REQUIRED)

# source files
set(SOURCES src/main.cpp)

# create executable
add_executable(gltf_renderer ${SOURCES})

target_include_directories(gltf_renderer PRIVATE "External Libraries")

target_link_libraries(gltf_renderer glad ${GLFW_LIBRARY} OpenGL32)