cmake_minimum_required(VERSION 3.15.0)
project(gltf_renderer)

# set standerd
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# include externals
#[[set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)
]]#
#include_directories(external/glad/include)
#include_directories(external/)

include_directories(external/glfw/include)

set(GLFW_LIBRARY external/glfw/lib-vc2022/glfw3.lib)

#find_package(glfw3 3.3 REQUIRED)

# source files
set(SOURCES src/main.cpp) #external/glad/src/glad.c)

# create executable
add_executable(gltf_renderer ${SOURCES})

target_include_directories(gltf_renderer PRIVATE "External Libraries")

target_link_libraries(gltf_renderer ${GLFW_LIBRARY} OpenGL32)