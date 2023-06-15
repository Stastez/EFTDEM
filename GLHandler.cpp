#include "GLHandler.h"
#include "DataStructures.h"

#include <iostream>
#include <fstream>
#include <magic_enum.hpp>

using namespace gl;

void GLAPIENTRY MessageCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
             ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
             type, severity, message );
}

GLFWwindow * GLHandler::initializeGL(bool debug) {
    if (isInitialized(debug)) uninitializeGL();

    if (!magic_enum::is_magic_enum_supported) {
        std::cout << "Current compiler does not support magic_enum!" << std::endl;
        exit(5);
    }

    if(!glfwInit()) {
        std::cout << "Could not initialize GLFW." << std::endl;
        exit(4);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, debug);
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

    ssbos = std::vector<GLuint>(numBuffers);
    glGenBuffers(numBuffers, ssbos.data());

    coherentBufferMask = std::vector<bool>(numBuffers + 1, false);

    initialized = true;
    this->isDebug = debug;

    return context;
}

void GLHandler::uninitializeGL() {
    glDeleteBuffers(numBuffers, ssbos.data());
    glfwTerminate();
    initialized = false;
}

std::vector<GLuint> GLHandler::getShaderPrograms(const std::vector<std::string>& shaderFiles) {
    std::vector<GLuint> programs;

    for (const auto& shaderFile : shaderFiles) {
        std::ifstream shaderFileStream;
        std::stringstream shaderStream;
        shaderFileStream.open(shaderFile);
        if (!shaderFileStream.is_open()) {
            std::cout << "Specified shader could not be opened: " << shaderFile << std::endl;
            exit(3);
        }
        shaderStream << shaderFileStream.rdbuf();
        auto shaderString = shaderStream.str();
        replaceBufferPlaceholders(shaderString);
        auto shader = shaderString.c_str();

        int success;
        char infoLog[512];
        auto shaderNumber = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(shaderNumber, 1, &shader, nullptr);
        glCompileShader(shaderNumber);
        glGetShaderiv(shaderNumber, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shaderNumber, 512, nullptr, infoLog);
            std::cout << infoLog << std::endl;
            exit(4);
        }

        auto program = glCreateProgram();
        glAttachShader(program, shaderNumber);

        glLinkProgram(program);

        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cout << infoLog << std::endl;
            exit(4);
        }

        glDeleteShader(shaderNumber);

        programs.emplace_back(program);
    }

    return programs;
}

void GLHandler::bindBuffer(GLHandler::bufferIndices buffer) {
    if (buffer == EFTDEM_UNBIND) glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    else glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer, ssbos[buffer - 1]); //no buffer needed for EFTDEM_UNBIND
}

void GLHandler::dataToBuffer(GLHandler::bufferIndices buffer, gl::GLsizeiptr size, const void *data, gl::GLenum usage) {
    if (buffer == EFTDEM_UNBIND) exit(2);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer, ssbos[buffer - 1]); //no buffer needed for EFTDEM_UNBIND
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
    coherentBufferMask[buffer] = true;
}

void GLHandler::dataFromBuffer(GLHandler::bufferIndices buffer, gl::GLsizeiptr offset, gl::GLsizeiptr size, void *data) {
    if (buffer == EFTDEM_UNBIND) exit(2);
    bindBuffer(buffer);
    waitForShaderStorageIntegrity();
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
}

std::vector<gl::GLuint> GLHandler::getBuffers() {
    return ssbos;
}

void GLHandler::setProgram(gl::GLuint program) {
    glUseProgram(program);
    currentProgram = program;
}

gl::GLuint GLHandler::getProgram() const {
    return currentProgram;
}

void GLHandler::waitForShaderStorageIntegrity() {
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

std::vector<bool> GLHandler::getCoherentBufferMask() {
    return coherentBufferMask;
}
bool GLHandler::isInitialized() const {return isInitialized(false) || isInitialized(true);}
bool GLHandler::isInitialized(bool debug) const {return initialized && (this->isDebug == debug);}

void GLHandler::dispatchShader(unsigned int shader, unsigned int localBatchSize, unsigned long resolutionX, unsigned long resolutionY){
    std::chrono::time_point<std::chrono::steady_clock> startInvocation, endInvocation;

    if (localBatchSize == 0) {
        unsigned int batchSize = 1;

        // find optimal batch size
        while (duration_cast<std::chrono::milliseconds>(endInvocation - startInvocation) <
               std::chrono::milliseconds{500}) {
            startInvocation = std::chrono::high_resolution_clock::now();
            glUniform2ui(glGetUniformLocation(shader, "currentInvocation"), batchSize, batchSize);
            glDispatchCompute(batchSize, batchSize, 1);
            auto sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
            glDeleteSync(sync);

            endInvocation = std::chrono::high_resolution_clock::now();

            batchSize *= 2;
        }

        localBatchSize = batchSize;
    }

    bool first = true;
    unsigned int totalInvocations = std::ceil(resolutionX * resolutionY), currentInvocation;
    for (unsigned long batchX = 0; batchX < resolutionX; batchX += localBatchSize * 8) {
        for (unsigned long batchY = 0; batchY < resolutionY; batchY += localBatchSize * 4) {
            glUniform2ui(glGetUniformLocation(shader, "currentInvocation"), batchX, batchY);
            glDispatchCompute(localBatchSize, localBatchSize, 1);
            auto sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

            currentInvocation = batchX * resolutionY + batchY;
            if (!first) {
                std::cout << "\x1b[2K"; // Delete current line
                for (int i = 0; i < 5; i++) {
                    std::cout
                            << "\x1b[1A" // Move cursor up one
                            << "\x1b[2K"; // Delete the entire line
                }
                std::cout << "\r";
            }
            first = false;

            endInvocation = std::chrono::high_resolution_clock::now();
            auto elapsedTime = endInvocation - startInvocation;

            std::cout << "Batch number: " << currentInvocation
                      << " / " << totalInvocations - 1 << std::endl;
            std::cout << "Elapsed time for " << localBatchSize << " invocations: "
                      << duration_cast<std::chrono::milliseconds>(elapsedTime) << std::endl
                      << "Average elapsed time per invocation: "
                      << duration_cast<std::chrono::milliseconds>(elapsedTime) / localBatchSize
                      << std::endl;
            std::cout << "Batch size: " << localBatchSize << std::endl << std::endl;

            if (duration_cast<std::chrono::milliseconds>(endInvocation - startInvocation) > std::chrono::milliseconds {1000}) {
                localBatchSize /= 2;
                batchX = 0;
                batchY = 0;
            }

            startInvocation = std::chrono::high_resolution_clock::now();

            glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
            glDeleteSync(sync);
        }
    }
}

void GLHandler::replaceBufferPlaceholders(std::string &shaderSource) {
    for (auto i = 1; i < magic_enum::enum_count<bufferIndices>(); i++) {
        size_t index = 0;
        auto bufferName = magic_enum::enum_name<bufferIndices>(magic_enum::enum_value<bufferIndices>(i));

        while (true) {
            index = shaderSource.find(bufferName, index);
            if (index == std::string::npos) break;

            std::string replacement = std::to_string(i);
            shaderSource.erase(index, bufferName.length() - 1);
            shaderSource.replace(index, replacement.length(), replacement);

            index += replacement.length();
        }
    }
}