cmake_minimum_required(VERSION 3.15.0)
project(gltf_renderer)

# set standerd
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


include_directories(external/glfw/include)
#find_library(GLFW_LIBRARY glfw3 PATHS ${CMAKE_SOURCE_DIR}/external/glfw/lib-vc2022)
set(GLFW_LIBRARY ${CMAKE_SOURCE_DIR}/external/glfw/lib-vc2022/glfw3.lib)  # add glfw
include_directories(external/glad/include)  # add glad
add_library(glad STATIC external/glad/src/glad.c)
include_directories(${CMAKE_SOURCE_DIR}/external/glm)  # add glm
include_directories(${CMAKE_SOURCE_DIR}/external/gltf)  # add gltf

# source files
set(SOURCES src/main.cpp)

# create executable
add_executable(gltf_renderer ${SOURCES})

# add imgui
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
set(IMGUI_SOURCES
	${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_sources(gltf_renderer PRIVATE ${IMGUI_SOURCES})
target_include_directories(gltf_renderer PRIVATE ${IMGUI_DIR} ${CMAKE_SOURCE_DIR}/src)


target_link_libraries(gltf_renderer imgui glad ${GLFW_LIBRARY} OpenGL32)