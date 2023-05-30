#include "glHandler.h"
#include <glbinding/gl/gl.h>
#include <glbinding-aux/ContextInfo.h>
#include <glbinding/glbinding.h>

#include <iostream>
#include <fstream>

using namespace gl;

void GLAPIENTRY MessageCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
             ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
             type, severity, message );
}

GLFWwindow * glHandler::initializeGL() {
    if(!glfwInit()) {
        std::cout << "Could not initialize GLFW." << std::endl;
        exit(4);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    context = glfwCreateWindow(800, 600, "EFTDEM", nullptr, nullptr);
    if (context == nullptr) {
        const char* error;
        glfwGetError(&error);
        std::cout << "Failed to create GLFW window: " << error << std::endl;
        glfwTerminate();
        exit(4);
    }
    glfwMakeContextCurrent(context);

    glbinding::initialize(glfwGetProcAddress);
    std::cout << "OpenGL Renderer: " << glbinding::aux::ContextInfo::renderer() << std::endl;

    glDebugMessageCallback(MessageCallback, nullptr);

    return context;
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

    int success;
    char infoLog[512];
    auto shaderNumber = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shaderNumber, 1, &shader, nullptr);
    glCompileShader(shaderNumber);
    glGetShaderiv(shaderNumber, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shaderNumber, 512, nullptr, infoLog);
        std::cout << infoLog << std::endl;
        exit(4);
    };

    shaderProgram shaderProgram{.ID = glCreateProgram()};
    glAttachShader(shaderProgram.ID, shaderNumber);
    glLinkProgram(shaderProgram.ID);
    glGetProgramiv(shaderProgram.ID, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(shaderProgram.ID, 512, nullptr, infoLog);
        std::cout << infoLog << std::endl;
        exit(4);
    }

    glDeleteShader(shaderNumber);
    return shaderProgram;
}