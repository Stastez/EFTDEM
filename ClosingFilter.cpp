#include <iostream>
#include "ClosingFilter.h"
#include "DataStructures.h"
#include "GLHandler.h"

ClosingFilter::ClosingFilter(GLHandler *glHandler, unsigned int kernelRadius, unsigned int batchSize = 0) {
    ClosingFilter::glHandler = glHandler;
    ClosingFilter::kernelRadius = kernelRadius;
    ClosingFilter::batchSize = batchSize;
    ClosingFilter::stageUsesGPU = true;
}

void ClosingFilter::cleanUp() {

}

heightMap ClosingFilter::apply(heightMap *map, bool generateOutput) {
    using namespace gl;

    std::cout << "Applying closing filter using OpenGL..." << std::endl << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    bool debug = false;

    if (!glHandler->isInitialized(debug)) glHandler->initializeGL(debug);
    std::vector<std::string> shaderPaths;
    shaderPaths.emplace_back("../../shaders/dilation.glsl");
    shaderPaths.emplace_back("../../shaders/erosion.glsl");
    auto shader = glHandler->getShaderPrograms(shaderPaths);
    glUseProgram(shader[0]);


    if (!glHandler->getCoherentBufferMask()[GLHandler::EFTDEM_HEIGHTMAP_BUFFER]){
        glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                (long long) sizeof(double) * map->resolutionX * map->resolutionY,
                                map->heights.data(), GL_STATIC_DRAW);
    }
    glHandler->dataToBuffer(GLHandler::EFTDEM_DILATION_RESULT_BUFFER,
                            (long long) sizeof(double) * map->resolutionX * map->resolutionY,
                            nullptr, GL_STREAM_READ);
    glHandler->dataToBuffer(GLHandler::EFTDEM_FILLED_MAP_BUFFER,
                            (long long) sizeof(double) * map->resolutionX * map->resolutionY,
                            nullptr, GL_STREAM_READ);

    glUniform2ui(glGetUniformLocation(shader[0], "resolution"), map->resolutionX, map->resolutionY);
    glUniform1ui(glGetUniformLocation(shader[0], "kernelRadius"), kernelRadius);
    glHandler->dispatchShader(shader[0], batchSize, map->resolutionX, map->resolutionY);
    glHandler->waitForShaderStorageIntegrity();

    glUseProgram(shader[1]);
    glUniform2ui(glGetUniformLocation(shader[1], "resolution"), map->resolutionX, map->resolutionY);
    glUniform1ui(glGetUniformLocation(shader[1], "kernelRadius"), kernelRadius);
    glHandler->dispatchShader(shader[1], batchSize, map->resolutionX, map->resolutionY);

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for closing: " << duration_cast<std::chrono::milliseconds>(end - start) << std::endl;

    if (!generateOutput) return emptyHeightMapfromHeightMap(map);

    heightMap filledMap = emptyHeightMapfromHeightMap(map);
    glHandler->dataFromBuffer(GLHandler::EFTDEM_FILLED_MAP_BUFFER,
                              0,
                              (long long) sizeof(GLdouble) * filledMap.resolutionX * filledMap.resolutionY,
                              filledMap.heights.data());

    return filledMap;
}