#ifndef EFTDEM_GLHANDLER_H
#define EFTDEM_GLHANDLER_H

#include <GLFW/glfw3.h>
#include <glbinding/gl/gl.h>
#include <glbinding-aux/ContextInfo.h>
#include <glbinding/glbinding.h>
#include <string>
#include <magic_enum.hpp>

class GLHandler {
public:
    enum bufferIndices {
        EFTDEM_UNBIND,
        EFTDEM_RAW_POINT_BUFFER, // 3 double for each point
        EFTDEM_RAW_POINT_INDEX_BUFFER, // unsigned int for each point
        EFTDEM_SORTED_POINT_SUM_BUFFER, // unsigned int for each grid cell
        EFTDEM_SORTED_POINT_COUNT_BUFFER, // unsigned int for each grid cell
        EFTDEM_HEIGHTMAP_BUFFER, // double for each grid cell
        EFTDEM_DILATION_HORIZONTAL_SUM_BUFFER, // double for each grid cell
        EFTDEM_DILATION_HORIZONTAL_AMOUNT_BUFFER, // unsigned int for each grid cell
        EFTDEM_DILATION_RESULT_BUFFER, // double for each grid cell
        EFTDEM_EROSION_HORIZONTAL_AMOUNT_BUFFER, // unsigned int for each grid cell
        EFTDEM_FILLED_MAP_BUFFER // double for each grid cell
    };

    GLFWwindow * initializeGL(bool debug);
    void uninitializeGL();

    std::vector<gl::GLuint> getShaderPrograms(const std::vector<std::string>& shaderFiles);

    void bindBuffer(bufferIndices buffer);
    void dataToBuffer(bufferIndices buffer, gl::GLsizeiptr size, const void *data, gl::GLenum usage);
    void dataFromBuffer(bufferIndices buffer, gl::GLsizeiptr offset, gl::GLsizeiptr size, void *data);
    void dispatchShader(unsigned int shader, unsigned int localBatchSize, unsigned long resolutionX, unsigned long resolutionY);
    void waitForShaderStorageIntegrity();
    std::vector<gl::GLuint> getBuffers();

    void setProgram(gl::GLuint program);
    gl::GLuint getProgram() const;

    bool isInitialized() const;
    bool isInitialized(bool debug) const;
    std::vector<bool> getCoherentBufferMask();

private:
    GLFWwindow* context;
    bool initialized = false;
    bool isDebug;

    std::vector<bool> coherentBufferMask;
    std::vector<gl::GLuint> ssbos;
    gl::GLuint currentProgram;

    int numBuffers = magic_enum::enum_count<bufferIndices>() - 1;

    void replaceBufferPlaceholders(std::string &shaderSource);
};


#endif //EFTDEM_GLHANDLER_H
