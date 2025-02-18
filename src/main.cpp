#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>
#include "imgui.h"
#include "camera.h"
#include "shader.h"
#include <glm/gtx/string_cast.hpp>

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

typedef Camera MyCamera;

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;
bool firstMouse = true; // avoid initalial stutter

// forward declarations
void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// set camera
MyCamera camera(
    glm::vec3(0.f, 0.f, 5.f),
    glm::vec3(0.f, 1.f, 0.f),
    0.f,
    0.f
);

// initialize relevant variables
float deltaTime = 0.f;
float lastFrame = 0.f;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 1.0f;


// tinygltf loader functions
bool loadModel(tinygltf::Model& model, const char* filename) {
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    if (!warn.empty()) {
        std::cout << "WARNING: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cout << "ERR: " << warn << std::endl;
    }

    if (!res)
        std::cout << "Failed to load glTF: " << filename << std::endl;
    else
        std::cout << "Loaded glTF: " << filename << std::endl;

    return res;
}

//bind mesh
void bindMesh(std::map<int, GLuint>& vbos, tinygltf::Model& model, tinygltf::Mesh& mesh) {
    for (size_t i = 0; i < model.bufferViews.size(); ++i) {
        const tinygltf::BufferView& bufferView = model.bufferViews[i];

        // check if bufferview supported
        if (bufferView.target == 0) {
            std::cout << "WARN: bufferView.target is zero" << std::endl;
            continue;
        }

        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
        std::cout << "bufferview.target " << bufferView.target << std::endl;

        GLuint vbo;
        glGenBuffers(1, &vbo);
        vbos[i] = vbo;
        glBindBuffer(bufferView.target, vbo);

        std::cout << "buffer.data.size = " << buffer.data.size() << ", bufferview.byteOffset = " << bufferView.byteOffset << std::endl;

        glBufferData(bufferView.target, bufferView.byteLength, &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
    }

    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        tinygltf::Primitive primitive = mesh.primitives[i];
        tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

        for (auto& attrib : primitive.attributes) {
            tinygltf::Accessor accessor = model.accessors[attrib.second];
            int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
            glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

            int size = 1;
            if (accessor.type != TINYGLTF_TYPE_SCALAR) {
                size = accessor.type;
            }

            int vaa = -1;
            if (attrib.first.compare("POSITION") == 0) vaa = 0;
            if (attrib.first.compare("NORMAL") == 0) vaa = 1;
            if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 2;
            if (attrib.first.compare("TANGENT") == 0) vaa = 3;
            if (vaa > -1) {
                glEnableVertexAttribArray(vaa);
                glVertexAttribPointer(vaa, size, accessor.componentType, accessor.normalized ? GL_TRUE : GL_FALSE, byteStride, BUFFER_OFFSET(accessor.byteOffset));
            }
            else {
                std::cout << "vaa missing: " << attrib.first << std::endl;
            }
        }
    }
}

void bindModelNodes(std::map<int, GLuint>& vbos, tinygltf::Model& model, tinygltf::Node& node) {
    if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
        bindMesh(vbos, model, model.meshes[node.mesh]);
    }

    for (size_t i = 0; i < node.children.size(); i++) {
        assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
        bindModelNodes(vbos, model, model.nodes[node.children[i]]);
    }
}

std::pair<GLuint, std::map<int, GLuint>> bindModel(tinygltf::Model& model) {
    std::map<int, GLuint> vbos;
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    const tinygltf::Scene& scene = model.scenes[model.defaultScene];
    for (size_t i = 0; i < scene.nodes.size(); ++i) {
        assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
        bindModelNodes(vbos, model, model.nodes[scene.nodes[i]]);
    }

    glBindVertexArray(0);
    // clean vbos but not delete index/element buffers yet
    for (auto it = vbos.cbegin(); it != vbos.cend();) {
        tinygltf::BufferView bufferView = model.bufferViews[it->first];
        if (bufferView.target != GL_ELEMENT_ARRAY_BUFFER) {
            glDeleteBuffers(1, &vbos[it->first]);
            vbos.erase(it++);
        }
        else {
            ++it;
        }
    }

    return { vao, vbos };
}

void DrawMesh(const std::map<int, GLuint>& vbos, tinygltf::Model& model, tinygltf::Mesh& mesh) {
    for (size_t i = 0; i < mesh.primitives.size(); i++) {
        tinygltf::Primitive primitive = mesh.primitives[i];
        tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));
        glDrawElements(primitive.mode, indexAccessor.count, indexAccessor.componentType, BUFFER_OFFSET(indexAccessor.byteOffset));
    }
}

void drawModelNodes(const std::pair<GLuint, std::map<int, GLuint>>& vaoAndEbos, tinygltf::Model& model, tinygltf::Node& node) {
    if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
        DrawMesh(vaoAndEbos.second, model, model.meshes[node.mesh]);
    }
    for (size_t i = 0; i < node.children.size(); i++) {
        drawModelNodes(vaoAndEbos, model, model.nodes[node.children[i]]);
    }
}

void DrawModel(const std::pair<GLuint, std::map<int, GLuint>>& vaoAndEbos, tinygltf::Model& model) {
    glBindVertexArray(vaoAndEbos.first);

    const tinygltf::Scene& scene = model.scenes[model.defaultScene];
    for (size_t i = 0; i < scene.nodes.size(); ++i) {
        drawModelNodes(vaoAndEbos, model, model.nodes[scene.nodes[i]]);
    }

    glBindVertexArray(0);
}

void DebugModel(tinygltf::Model& model) {
    for (auto& mesh : model.meshes) {
        std::cout << "mesh : " << mesh.name << std::endl;
        for (auto& primitive : mesh.primitives) {
            const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];

            std::cout << "indexAccessor: count " << indexAccessor.count << ", type " << indexAccessor.componentType << std::endl;

            tinygltf::Material& mat = model.materials[primitive.material];
            for (auto& mats : mat.values) {
                std::cout << "mat : " << mats.first.c_str() << std::endl;
            }

            for (auto& image : model.images) {
                std::cout << "image name : " << image.uri << std::endl;
                std::cout << "size : " << image.image.size() << std::endl;
                std::cout << "w/h : " << image.width << "/" << image.height << std::endl;
            }

            std::cout << "indices : " << primitive.indices << std::endl;
            std::cout << "mode : " << "(" << primitive.mode << ")" << std::endl;

            for (auto& attrib : primitive.attributes) {
                std::cout << "attribute : " << attrib.first.c_str() << std::endl;
            }
        }
    }
}


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

    // gl settings
    glEnable(GL_DEPTH_TEST);

    // init shader
    Shader shader = Shader("../../../shaders/vertex_shader.txt", "../../../shaders/fragment_shader.txt");

    // load tinygltf model
    std::string filename = "../../../models/Cube/Cube.gltf";
    tinygltf::Model model;
    loadModel(model, filename.c_str());
    // model info
    std::cout << "n meshes: " << model.meshes.size() << std::endl;
    std::cout << "n nodes: " << model.nodes.size() << std::endl;
    std::cout << "number materials: " << model.materials.size() << std::endl;

    // call other load methods
    DebugModel(model);
    std::pair<GLuint, std::map<int, GLuint>> vaoAndEbos = bindModel(model);

    glm::vec3 sun_position = glm::vec3(3.0, 10.0, -5.0);
    glm::vec3 sun_color = glm::vec3(1.0);


    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), ((float)WIDTH / (float)HEIGHT), 0.1f, 100000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model_t = glm::mat4(1.0f);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setMat4("model", model_t);

        DrawModel(vaoAndEbos, model);

        //std::cout << "position: " << glm::to_string(camera.Position) << ", yaw: " << camera.Yaw << std::endl;
        std::cout << "position: " << glm::to_string(camera.Position) << ", yaw: " << camera.Yaw << ", pitch: " << camera.Pitch << std::endl;

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    // clear all buffers

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);


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