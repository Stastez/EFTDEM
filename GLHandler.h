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
        EFTDEM_RAW_POINT_BUFFER,
        EFTDEM_RAW_POINT_SUM_BUFFER,
        EFTDEM_RAW_POINT_COUNT_BUFFER,
        EFTDEM_SORTED_POINTS_BUFFER,
        EFTDEM_SORTED_POINTS_OFFSET_BUFFER,
        EFTDEM_HEIGHTMAP_BUFFER,
        EFTDEM_SECOND_HEIGHTMAP_BUFFER,
        EFTDEM_FIRST_CLOSING_FILTER_BUFFER,
        EFTDEM_SECOND_CLOSING_FILTER_BUFFER,
        EFTDEM_THIRD_CLOSING_FILTER_BUFFER
    };

    GLFWwindow * initializeGL(bool debug);
    void uninitializeGL();

    std::vector<gl::GLuint> getShaderPrograms(std::vector<std::string> shaderFiles);

    void bindBuffer(bufferIndices buffer);
    void dataToBuffer(bufferIndices buffer, gl::GLsizeiptr size, const void *data, gl::GLenum usage);
    void dataFromBuffer(bufferIndices buffer, gl::GLsizeiptr offset, gl::GLsizeiptr size, void *data);

    void setProgram(gl::GLuint program);

    bool isInitialized();
    bool isInitialized(bool debug);

    static const unsigned int bufferCount = 11;
    gl::GLuint ssbos[bufferCount];

private:
    GLFWwindow* context;
    bool initialized = false;
    bool isDebug;

    gl::GLuint currentProgram;
};


#endif //EFTDEM_GLHANDLER_H
