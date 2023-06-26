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
        EFTDEM_HORIZONTAL_SUM_BUFFER, // unsigned int for each grid cell
        EFTDEM_SUM_BUFFER, // double for each grid cell
        EFTDEM_HORIZONTAL_AMOUNT_BUFFER, // unsigned int for each grid cell
        EFTDEM_AMOUNT_BUFFER, // unsigned int for each grid cell
        EFTDEM_AVERAGE_BUFFER, // double for each grid cell
        EFTDEM_CLOSING_MASK_BUFFER, // double for each grid cell
        EFTDEM_INTERIM_RESULT_BUFFER, // double for each grid cell
        EFTDEM_TOTAL_WEIGHT_BUFFER, // double for each grid cell
    };

    explicit GLHandler(std::string shaderDirectory);

    GLFWwindow * initializeGL(bool debug);
    void uninitializeGL();

    std::vector<gl::GLuint> getShaderPrograms(const std::vector<std::string>& shaderFiles, bool useStandardDirectory);

    void bindBuffer(bufferIndices buffer);
    void generateBuffer(bufferIndices buffer);
    void generateBuffer(int buffer);
    void deleteBuffer(bufferIndices buffer);
    void deleteBuffer(int buffer);
    void dataToBuffer(bufferIndices buffer, gl::GLsizeiptr size, const void *data, gl::GLenum usage);
    void dataFromBuffer(bufferIndices buffer, gl::GLsizeiptr offset, gl::GLsizeiptr size, void *data);
    void dispatchShader(unsigned int localBatchSize, unsigned long resolutionX, unsigned long resolutionY) const;
    static void waitForShaderStorageIntegrity();

    void setProgram(gl::GLuint program);
    [[nodiscard]] gl::GLuint getProgram() const;

    [[nodiscard]] bool isInitialized(bool debug) const;
    std::vector<bool> getCoherentBufferMask();

    std::string getShaderDir();

private:
    GLFWwindow* context{};
    bool initialized = false;
    bool isDebug{};
    std::string shaderDirectory;

    std::vector<bool> coherentBufferMask;
    std::vector<bool> deletedBufferMask;
    std::vector<gl::GLuint> ssbos;
    gl::GLuint currentProgram{};

    unsigned long long numBuffers = magic_enum::enum_count<bufferIndices>() - 1;

    static void replaceBufferPlaceholders(std::string &shaderSource);
};


#endif //EFTDEM_GLHANDLER_H
