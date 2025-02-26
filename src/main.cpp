#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>
#include "imgui.h"
#include "nlohmann/json.hpp"
#include <fstream>

#include "camera.h"

using json = nlohmann::json;

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;
bool firstMouse = true; // avoid initalial stutter

// forward declarations
void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// set camera
Camera camera(
    glm::vec3(0.f, 0.f, 0.f),
    glm::vec3(0.f, 0.f, 1.f),
    0.f,
    0.f
);

// initialize relevant variables
float deltaTime = 0.f;
float lastFrame = 0.f;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 1.0f;


int main()
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "glTF renderer", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    // glfw settings
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // initialize glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "failed to init GLAD" << std::endl;
        return -1;
    }

    // parse json TODO put in seperate function
    std::ifstream file("../../../models/cube/Cube.gltf");
    if (!file.is_open()) {
        std::cerr << "failed to load gltf json" << std::endl;
        return -1;
    }
    json data = json::parse(file);
    file.close();

    // loop all nodes
    for (auto& node : data["nodes"])
    {
        int mesh_index = node["mesh"];
        //std::cout << "mesh: " << node["mesh"] << std::endl;
        json meshData = data["meshes"][mesh_index];
        std::vector<json> primitives = meshData["primitives"];
        for (json& primitive : primitives)
        {
            int mode = primitive["mode"];  // mode indicates whether to render as POINTS/TRIANGLES/LINES etc
            int primitive_accessor = primitive["indices"];
            int primitive_material_accessor = primitive["material"];
        }
    }

    std::cout << data["accessors"];

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();

        //std::cout << "windowshouls close: " << glfwWindowShouldClose(window);
    }

    // clear all buffers

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // used for dissapear and re appear mouse. RREMOVE if not needed
    /*if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        if (!glfw_cursor_normal) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfw_cursor_normal = true;
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfw_cursor_normal = false;
        }*/


    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

}

void framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}