#include "ClosingFilter_Old.h"
#include "DataStructures.h"
#include "GLHandler.h"
#include <iostream>

ClosingFilter_Old::ClosingFilter_Old(GLHandler *glHandler, unsigned int kernelRadius, unsigned int batchSize = 0) {
    ClosingFilter_Old::glHandler = glHandler;
    ClosingFilter_Old::kernelRadius = kernelRadius;
    ClosingFilter_Old::batchSize = batchSize;
    ClosingFilter_Old::stageUsesGPU = true;
}

ClosingFilter_Old::~ClosingFilter_Old() noexcept = default;

void ClosingFilter_Old::allocBuffer(GLHandler::bufferIndices buffer, long singleDataSize, long dataCount) {
    glHandler->dataToBuffer(buffer, singleDataSize * dataCount, nullptr, gl::GL_STREAM_READ);
}

heightMap ClosingFilter_Old::apply(heightMap *map, bool generateOutput) {
    using namespace gl;

    std::cout << "Applying closing filter using OpenGL..." << std::endl << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::string> shaderPaths;
    shaderPaths.emplace_back("discretization.glsl");
    shaderPaths.emplace_back("horizontalAmount.glsl");
    shaderPaths.emplace_back("amount.glsl");
    shaderPaths.emplace_back("horizontalSum.glsl");
    shaderPaths.emplace_back("sum.glsl");
    shaderPaths.emplace_back("average.glsl");

    shaderPaths.emplace_back("discretization.glsl");
    shaderPaths.emplace_back("horizontalAmount.glsl");
    shaderPaths.emplace_back("amount.glsl");
    shaderPaths.emplace_back("dilation.glsl");
    shaderPaths.emplace_back("horizontalAmount.glsl");
    shaderPaths.emplace_back("amount.glsl");
    shaderPaths.emplace_back("erosion.glsl");

    shaderPaths.emplace_back("closing.glsl");

    auto shader = glHandler->getShaderPrograms(shaderPaths, true);

    auto pixelCount = (long) (map->resolutionX * map->resolutionY);

    glHandler->setProgram(shader[0]);

    if (!glHandler->getCoherentBufferMask()[GLHandler::EFTDEM_HEIGHTMAP_BUFFER]){
        glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                (long) (sizeof(double) * pixelCount),
                                map->heights.data(), GL_STATIC_DRAW);
    }

    auto bufferSpecs = std::vector<std::vector<bufferSpecifications>>(shader.size());
    // discretization
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_CLOSING_MASK_BUFFER, sizeof(double)}});
    // horizontalAmount
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_HORIZONTAL_AMOUNT_BUFFER, sizeof(unsigned)}});
    // amount
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_AMOUNT_BUFFER, sizeof(unsigned)}});
    // horizontalSum
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_HORIZONTAL_SUM_BUFFER, sizeof(double)}});
    // sum
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_SUM_BUFFER, sizeof(double)}});
    // average
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_AVERAGE_BUFFER, sizeof(double)}});
    //repeat
    for (int i = 0; i < 3; i++) bufferSpecs.emplace_back();
    // dilation
    bufferSpecs.emplace_back();
    // horizontalAmount again
    bufferSpecs.emplace_back();
    // amount again
    bufferSpecs.emplace_back();
    // erosion
    bufferSpecs.emplace_back();
    // closing
    bufferSpecs.emplace_back();

    for (auto i = 0ul; i < bufferSpecs.size(); i++) {
        for (auto spec : bufferSpecs[i]) allocBuffer(spec.buffer, spec.elementSize, pixelCount);

        glHandler->setProgram(shader[i]);
        glUniform2ui(glGetUniformLocation(shader[i], "resolution"), map->resolutionX, map->resolutionY);
        glUniform1ui(glGetUniformLocation(shader[i], "kernelRadius"), kernelRadius);
        glHandler->dispatchShader(batchSize, map->resolutionX, map->resolutionY);
        GLHandler::waitForShaderStorageIntegrity();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for closing: " << duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    if (!generateOutput) return emptyHeightMapfromHeightMap(map);

    heightMap filledMap = emptyHeightMapfromHeightMap(map);
    glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                              0,
                              (long) (sizeof(GLdouble) * filledMap.resolutionX * filledMap.resolutionY),
                              filledMap.heights.data());

    return filledMap;
}