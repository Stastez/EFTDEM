#include "GLHandler.h"
#include "DataStructures.h"
#include "Pipeline.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>
#include <magic_enum.hpp>
#include <utility>

using namespace gl;

GLHandler::GLHandler() = default;

/**
 * Constructs a GLHandler that will prepend shaderDirectory to all given shader paths if later calls request this.
 * @param shaderDirectory The path to the default shader directory
 */
GLHandler::GLHandler(std::string shaderDirectory) {
    GLHandler::shaderDirectory = std::move(shaderDirectory);
}

/**
 * Destroys the GLHandler, deletes all buffers and terminates GLFW.
 */
GLHandler::~GLHandler() {
    if (!initialized) return;

    for (unsigned long long i = 1; i <= numBuffers; i++) {
        if (!deletedBufferMask.at(i)) deleteBuffer((int) i);
    }

    glfwTerminate();
    context = nullptr;
    initialized = false;
}

/**
 * OpenGL callback for errors for example during shader compilation.
 */
void GLAPIENTRY MessageCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    // must conform to GL specified template
    (void) source; (void) type; (void) id; (void) severity; (void) length; (void) userParam;

    fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
             ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
             (unsigned int) type, (unsigned int) severity, message );
}

/**
 * Initializes the GLHandler by creating a GLFW OpenGL context as well as a glbinding context. This will also generate
 * all buffers contained in GLHandler::bufferIndices and set up buffer management. The program may exit with EXIT_OPENGL_ERROR
 * for any of these reasons:
 * <ul>
 *  <li> GLFW could not initialize
 *  <li> GLFW could not create a window with the specified parameters
 * </ul>
 * @param debug Whether or not to create a debug OpenGL context. Usually you want this to be false.
 * @return A pointer to the created GLFWwindow (i.e., the OpenGL context)
 */
GLFWwindow * GLHandler::initializeGL(bool debug = false) {
    if (isInitialized(debug)) return context;

    if (!magic_enum::is_magic_enum_supported) {
        std::cout << "Current compiler does not support magic_enum!" << std::endl;
        exit(Pipeline::EXIT_DEPENDENCY_ERROR);
    }

    if(!glfwInit()) {
        std::cout << "Could not initialize GLFW." << std::endl;
        exit(Pipeline::EXIT_OPENGL_ERROR);
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
        exit(Pipeline::EXIT_OPENGL_ERROR);
    }
    glfwMakeContextCurrent(context);

    glbinding::useCurrentContext();
    glbinding::initialize(glfwGetProcAddress);
    std::cout << "OpenGL Renderer: " << glbinding::aux::ContextInfo::renderer() << std::endl;

    glDebugMessageCallback(MessageCallback, nullptr);

    ssbos = std::vector<GLuint>(numBuffers);
    glGenBuffers((int) numBuffers, ssbos.data());

    coherentBufferMask = std::vector<bool>(numBuffers + 1, false);
    deletedBufferMask = std::vector<bool>(numBuffers + 1, false);

    initialized = true;
    this->isDebug = debug;
    
    return context;
}

/**
 * Get the shader program compiled from the GLSL file pointed to by shaderFile. If useStandardDirectory is true,
 * GLHandler::shaderDirectory is prepended on shaderFile. The program may exit if the requested shaderFile could not
 * be opened.
 * @param shaderFile The path to the requested GLSL file
 * @param useStandardDirectory Whether or not to prepend GLHandler::shaderDirectory on shaderFile
 * @throws std::exception If there was an error compiling or linking the requested shader
 * @return The OpenGL object number of the compiled shader
 */
gl::GLuint GLHandler::getShaderProgram(const std::string& shaderFile, bool useStandardDirectory){
    std::ifstream shaderFileStream;
    std::stringstream shaderStream;

    if (useStandardDirectory) shaderFileStream.open(shaderDirectory + "/" + shaderFile);
    else shaderFileStream.open(shaderFile);

    if (!shaderFileStream.is_open()) {
        if (useStandardDirectory) std::cout << "Specified shader could not be opened: " << shaderDirectory << "/" << shaderFile << std::endl;
        else std::cout << "Specified shader could not be opened: " << shaderFile << std::endl;
        exit(Pipeline::EXIT_IO_ERROR);
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
        std::cout << "ERROR -- " << shaderFile << " -- ERROR" << std::endl;
        glGetShaderInfoLog(shaderNumber, 512, nullptr, infoLog);
        std::cout << infoLog << std::endl;
        throw std::exception();
    }

    auto program = glCreateProgram();
    glAttachShader(program, shaderNumber);

    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        std::cout << "ERROR -- " << shaderFile << " -- ERROR" << std::endl;
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cout << infoLog << std::endl;
        throw std::exception();
    }

    glDeleteShader(shaderNumber);

    return program;
}

/**
 * Get the shader programs compiled from the GLSL files pointed to by shaderFiles. If useStandardDirectory is true,
 * GLHandler::shaderDirectory is prepended on every string in shaderFiles. The program may exit if any of the requested shaderFiles could
 * not be opened.
 * @param shaderFiles A vector of paths to the requested GLSL files
 * @param useStandardDirectory Whether or not to prepend GLHandler::shaderDirectory on every string in shaderFiles
 * @throws std::exception If there was an error compiling or linking any of the requested shaders
 * @return A vector of OpenGL object numbers of the compiled shaders
 */
std::vector<GLuint> GLHandler::getShaderPrograms(const std::vector<std::string>& shaderFiles, bool useStandardDirectory) {
    std::vector<GLuint> programs;

    for (const auto& shaderFile : shaderFiles) {
        GLuint program = getShaderProgram(shaderFile, useStandardDirectory);
        programs.emplace_back(program);
    }

    return programs;
}

/**
 * Binds the specified buffer to its specific binding point or unbinds the buffer target if buffer is 0.
 * @param buffer The buffer to be bound
 * @throws std::exception If the requested buffer was previously deleted
 */
void GLHandler::bindBuffer(GLHandler::bufferIndices buffer) {
    if (deletedBufferMask.at(buffer)) {
        std::cout << magic_enum::enum_name(buffer) << " was previously deleted." << std::endl;
        throw std::exception();
    }
    if (buffer == EFTDEM_UNBIND) glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    else glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer, ssbos.at(buffer - 1)); //no buffer needed for EFTDEM_UNBIND
}

/**
 * Generates the specified buffer. Also see generateBuffer(int).
 * @param buffer The bufferIndices member to generate
 */
void GLHandler::generateBuffer(GLHandler::bufferIndices buffer) {
    generateBuffer((int) buffer);
}

/**
 * Generates the specified buffer. Does nothing if the buffer already exists.
 * @throws std::exception When the provided buffer index is equal to 0
 * @param buffer The number of the buffer to generate. Equates to a bufferIndices member
 */
void GLHandler::generateBuffer(int buffer) {
    if (!deletedBufferMask.at(buffer)) return;
    if (buffer == EFTDEM_UNBIND) throw std::exception();
    glGenBuffers(1, &ssbos.at(buffer - 1));
    glFlush();
    deletedBufferMask.at(buffer) = false;
    coherentBufferMask.at(buffer) = false;
}

/**
 * Deletes the specified buffer. Also see deleteBuffer(int).
 * @param buffer The bufferIndices member to delete
 */
void GLHandler::deleteBuffer(GLHandler::bufferIndices buffer) {
    deleteBuffer((int) buffer);
}

/**
 * Deletes the specified buffer. Does nothing if the buffer was deleted previously.
 * @throws std::exception When the provided buffer index is equal to 0
 * @param buffer The number of the buffer to delete. Equates to a bufferIndices member
 */
void GLHandler::deleteBuffer(int buffer) {
    if (deletedBufferMask.at(buffer)) return;
    if (buffer == EFTDEM_UNBIND) exit(Pipeline::EXIT_INVALID_FUNCTION_PARAMETERS);
    glDeleteBuffers(1, &ssbos.at(buffer - 1));
    glFlush();
    deletedBufferMask.at(buffer) = true;
    coherentBufferMask.at(buffer) = false;
}

/**
 * Copies <b>size</b> bytes of <b>data</b> to the buffer specified by <b>buffer</b>. The buffer will be created if it
 * did not exist previously of if it had already been deleted. <b>usage</b> will be passed to OpenGL, some GPU drivers
 * may behave differently according to this parameter.
 * @param buffer The buffer
 * @param size How many bytes to copy
 * @param data A pointer to the data to copy from
 * @param usage See https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBufferData.xhtml for reference
 */
void GLHandler::dataToBuffer(GLHandler::bufferIndices buffer, gl::GLsizeiptr size, const void *data, gl::GLenum usage) {
    if (deletedBufferMask.at(buffer)) generateBuffer(buffer);
    if (buffer == EFTDEM_UNBIND) exit(Pipeline::EXIT_INVALID_FUNCTION_PARAMETERS);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buffer, ssbos.at(buffer - 1)); //no buffer needed for EFTDEM_UNBIND
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usage);
    coherentBufferMask.at(buffer) = true;
}

/**
 * Copies the <b>size</b> bytes immediately after the first <b>offset</b> bytes from the buffer specified by <b>buffer</b>.
 * Calls waitForShaderStorageIntegrity to ensure the validity of the data.
 * @throws std::exception If the specified buffer was previously deleted
 * @param buffer The buffer
 * @param offset How many prefix bytes to ignore
 * @param size How many bytes to copy
 * @param data Where to copy the data from the buffer
 */
void GLHandler::dataFromBuffer(GLHandler::bufferIndices buffer, gl::GLsizeiptr offset, gl::GLsizeiptr size, void *data) {
    if (deletedBufferMask.at(buffer)) {
        std::cout << magic_enum::enum_name(buffer) << " was previously deleted." << std::endl;
        exit(Pipeline::EXIT_OPENGL_ERROR);
    }
    if (buffer == EFTDEM_UNBIND) exit(Pipeline::EXIT_INVALID_FUNCTION_PARAMETERS);
    bindBuffer(buffer);
    waitForShaderStorageIntegrity();
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
}

void GLHandler::setProgram(gl::GLuint program) {
    glUseProgram(program);
    currentProgram = program;
}

gl::GLuint GLHandler::getProgram() const {
    return currentProgram;
}

/**
 * Ensures that all compute shader invocations have finished writing data.
 */
void GLHandler::waitForShaderStorageIntegrity() {
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

std::vector<bool> GLHandler::getCoherentBufferMask() {
    return coherentBufferMask;
}

bool GLHandler::isInitialized(bool debug) const {return initialized && (this->isDebug == debug);}

/**
 * Executes the currently active compute shader program resolutionX * resolutionY times in batches of size localBatchSize.
 * If localBatchSize is 0, it will attempt to find a viable batch size automatically. This function is useful for time-intensive
 * calculations to prevent stalling the OpenGL command queue.
 * @param localBatchSize The batch size to use. 0 if automatic discovery is wished
 * @param resolutionX The amount of pixels to process in x-direction
 * @param resolutionY The amount of pixels to process in y-direction
 */
void GLHandler::dispatchShader(unsigned int localBatchSize, unsigned long resolutionX, unsigned long resolutionY) const {
    const auto currentInvocationLocation = glGetUniformLocation(getProgram(), "currentInvocation");

    auto startInvocation = std::chrono::high_resolution_clock::now(), endInvocation = std::chrono::high_resolution_clock::now();

    if (localBatchSize == 0) {
        unsigned int batchSize = 1;

        // find optimal batch size
        while (duration_cast<std::chrono::milliseconds>(endInvocation - startInvocation) <
               std::chrono::milliseconds{500}) {
            startInvocation = std::chrono::high_resolution_clock::now();
            glUniform2ui(currentInvocationLocation,batchSize, batchSize);
            glDispatchCompute(batchSize, batchSize, 1);
            auto sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
            glDeleteSync(sync);

            endInvocation = std::chrono::high_resolution_clock::now();

            batchSize = std::min((unsigned long) batchSize * 2, std::max((unsigned long) std::ceil((double) resolutionX / 8.), (unsigned long) std::ceil((double) resolutionY / 4.)));
            if (batchSize == std::max((unsigned long) std::ceil((double) resolutionX / 8.), (unsigned long) std::ceil((double) resolutionY / 4.))) break;
        }

        localBatchSize = batchSize;
    }

    std::cout << "Shader number: " << getProgram() << " Batch size: " << localBatchSize << std::endl;

    bool multipleBatches = false;
    GLsync previousSync = nullptr;
    for (unsigned long batchX = 0; batchX < resolutionX; batchX += localBatchSize * 8) {
        for (unsigned long batchY = 0; batchY < resolutionY; batchY += localBatchSize * 4) {
            
            glUniform2ui(currentInvocationLocation, batchX, batchY);
            glDispatchCompute(localBatchSize, localBatchSize, 1);

            if (previousSync != nullptr) {
                multipleBatches = true;
                std::cout << ".";
                glClientWaitSync(previousSync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
                glDeleteSync(previousSync);
            }

            auto currentSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

            std::swap(previousSync, currentSync);
        }
    }

    if (multipleBatches) std::cout << std::endl;

    if (previousSync != nullptr) {
        glClientWaitSync(previousSync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
        glDeleteSync(previousSync);
    }
}

/**
 * Convenience function that takes a string and replaces all bufferIndices member names with their corresponding index
 * (e.g. "layout (binding = EFTDEM_HEIGHTMAP_BUFFER) ..." -> "layout (binding = 5) ...")
 * @param shaderSource The string in which to replace names
 */
void GLHandler::replaceBufferPlaceholders(std::string &shaderSource) {
    for (unsigned long i = 1; i < magic_enum::enum_count<bufferIndices>(); i++) {
        size_t index = 0;
        auto bufferName = magic_enum::enum_name<bufferIndices>(magic_enum::enum_value<bufferIndices>(i));

        while (true) {
            index = shaderSource.find(bufferName, index);
            if (index == std::string::npos) break;

            std::string replacement = std::to_string(i);

            if (replacement.length() > bufferName.length()) shaderSource.insert(index, replacement.length() - bufferName.length(), '*');
            shaderSource.erase(index, bufferName.length() - replacement.length());
            shaderSource.replace(index, replacement.length(), replacement);

            index += replacement.length();
        }
    }
}

std::string GLHandler::getShaderDir() { return shaderDirectory; }

void GLHandler::resetBuffers() {
    glDeleteBuffers((int) numBuffers, ssbos.data());
    glGenBuffers((int) numBuffers, ssbos.data());
    std::fill(deletedBufferMask.begin(), deletedBufferMask.end(), false);
    std::fill(coherentBufferMask.begin(), coherentBufferMask.end(), false);
}
