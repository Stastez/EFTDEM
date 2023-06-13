#ifndef EFTDEM_GLHANDLER_H
#define EFTDEM_GLHANDLER_H

#include <GLFW/glfw3.h>
#include <glbinding/gl/gl.h>
#include <glbinding-aux/ContextInfo.h>
#include <glbinding/glbinding.h>
#include <string>

class GLHandler {
public:
    enum bufferIndices {
        EFTDEM_UNBIND,
        EFTDEM_RAW_POINT_BUFFER, // 3 double for each point
        EFTDEM_RAW_POINT_INDEX_BUFFER, // unsigned int for each point
        EFTDEM_SORTED_POINT_SUM_BUFFER, // unsigned int for each grid cell
        EFTDEM_SORTED_POINT_COUNT_BUFFER, // unsigned int for each grid cell
        EFTDEM_HEIGHTMAP_BUFFER, // double for each grid cell
        lengthElementDoNotUse
    };

    GLFWwindow * initializeGL(bool debug);
    void uninitializeGL();

    std::vector<gl::GLuint> getShaderPrograms(std::vector<std::string> shaderFiles);

    void bindBuffer(bufferIndices buffer);
    void dataToBuffer(bufferIndices buffer, gl::GLsizeiptr size, const void *data, gl::GLenum usage);
    void dataFromBuffer(bufferIndices buffer, gl::GLsizeiptr offset, gl::GLsizeiptr size, void *data);
    void waitForShaderStorageIntegrity();
    std::vector<gl::GLuint> getBuffers();

    void setProgram(gl::GLuint program);
    gl::GLuint getProgram();

    bool isInitialized();
    bool isInitialized(bool debug);
    std::vector<bool> getCoherentBufferMask();

private:
    GLFWwindow* context;
    bool initialized = false;
    bool isDebug;

    std::vector<bool> coherentBufferMask;
    std::vector<gl::GLuint> ssbos;
    gl::GLuint currentProgram;
};


#endif //EFTDEM_GLHANDLER_H
