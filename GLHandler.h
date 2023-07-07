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
        EFTDEM_RAW_POINT_BUFFER, // 3 float for each point
        EFTDEM_RAW_POINT_INDEX_BUFFER, // unsigned int for each point
        EFTDEM_SORTED_POINT_SUM_BUFFER, // unsigned int for each grid cell
        EFTDEM_SORTED_POINT_COUNT_BUFFER, // unsigned int for each grid cell
        EFTDEM_HEIGHTMAP_BUFFER, // float for each grid cell
        EFTDEM_SECOND_HEIGHTMAP_BUFFER, // float for each grid cell
        EFTDEM_COMPARISON_BUFFER, // float for each grid cell
        EFTDEM_HORIZONTAL_SUM_BUFFER, // unsigned int for each grid cell
        EFTDEM_SUM_BUFFER, // float for each grid cell
        EFTDEM_HORIZONTAL_AMOUNT_BUFFER, // unsigned int for each grid cell
        EFTDEM_AMOUNT_BUFFER, // unsigned int for each grid cell
        EFTDEM_AVERAGE_BUFFER, // float for each grid cell
        EFTDEM_CLOSING_MASK_BUFFER, // float for each grid cell
        EFTDEM_TOTAL_WEIGHT_BUFFER, // float for each grid cell
        EFTDEM_HORIZONTAL_TOTAL_WEIGHT_BUFFER, // float for each grid cell
        EFTDEM_KERNEL_BUFFER, // float for as manny fields as Kernel Radius
    };

    GLHandler();
    explicit GLHandler(std::string shaderDirectory);
    ~GLHandler();

    GLFWwindow * initializeGL(bool debug);

    std::vector<gl::GLuint> getShaderPrograms(const std::vector<std::string>& shaderFile, bool useStandardDirectory);
    gl::GLuint getShaderProgram(const std::string& shaderFiles, bool useStandardDirectory);

    void bindBuffer(bufferIndices buffer);
    void resetBuffers();
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
