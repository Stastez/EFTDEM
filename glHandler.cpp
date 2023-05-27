#include "glHandler.h"
#include <glbinding/gl/gl.h>
#include <glbinding-aux/ContextInfo.h>
#include <glbinding/glbinding.h>

#include <iostream>
#include <fstream>

using namespace gl;

void glHandler::initializeGL() {
    if(!glfwInit()) {
        std::cout << "Could not initialize GLFW." << std::endl;
        exit(4);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    context = glfwCreateWindow(800, 600, "EFTDEM", nullptr, nullptr);
    if (context == nullptr)
    {
        const char* error;
        glfwGetError(&error);
        std::cout << "Failed to create GLFW window: " << error << std::endl;
        glfwTerminate();
        exit(4);
    }
    glfwMakeContextCurrent(context);

    glbinding::initialize(glfwGetProcAddress);

    std::cout << "OpenGL Renderer: " << glbinding::aux::ContextInfo::renderer() << std::endl;
}

shaderProgram glHandler::getShader(const std::string& shaderFile) {
    std::ifstream shaderFileStream;
    std::stringstream shaderStream;

    shaderFileStream.open(shaderFile);
    if (!shaderFileStream.is_open()) {
        std::cout << "Specified shader could not be opened: " << shaderFile << std::endl;
        exit(3);
    }
    shaderStream << shaderFileStream.rdbuf();
    auto shaderString = shaderStream.str();
    auto shader = shaderString.c_str();

    auto shaderNumber = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shaderNumber, 1, &shader, nullptr);
    glCompileShader(shaderNumber);

    int success;
    glGetShaderiv(shaderNumber, GL_COMPILE_STATUS, &success);
    if (!success) {
        char compilationInfo[512];
        glGetShaderInfoLog(shaderNumber, 512, nullptr, compilationInfo);
        std::cout << "Shader compilation failed: " << compilationInfo << std::endl;
    }

    shaderProgram shaderProgram{.ID = glCreateProgram()};
    glAttachShader(shaderProgram.ID, shaderNumber);
    glLinkProgram(shaderProgram.ID);

    glGetProgramiv(shaderProgram.ID, GL_LINK_STATUS, &success);
    if (!success) {
        char compilationInfo[512];
        glGetProgramInfoLog(shaderProgram.ID, 512, nullptr, compilationInfo);
        std::cout << "Shader program linkage failed: " << compilationInfo << std::endl;
    }

    glDeleteShader(shaderNumber);
    return shaderProgram;
}

void use(shaderProgram* shader) {
    glUseProgram(shader->ID);
}