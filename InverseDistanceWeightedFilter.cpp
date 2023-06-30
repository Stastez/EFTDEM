#include "InverseDistanceWeightedFilter.h"
#include "DataStructures.h"
#include "GLHandler.h"
#include <iostream>

InverseDistanceWeightedFilter::InverseDistanceWeightedFilter(GLHandler *glHandler, unsigned int kernelRadius, unsigned int batchSize = 0) {
    InverseDistanceWeightedFilter::glHandler = glHandler;
    InverseDistanceWeightedFilter::kernelRadius = kernelRadius;
    InverseDistanceWeightedFilter::batchSize = batchSize;
    InverseDistanceWeightedFilter::stageUsesGPU = true;
}

InverseDistanceWeightedFilter::~InverseDistanceWeightedFilter() {
    auto bufferMask = glHandler->getCoherentBufferMask();

    glHandler->deleteBuffer(GLHandler::EFTDEM_CLOSING_MASK_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_HORIZONTAL_AMOUNT_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_AMOUNT_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_HORIZONTAL_SUM_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_SUM_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_AVERAGE_BUFFER);
};

void InverseDistanceWeightedFilter::allocBuffer(GLHandler::bufferIndices buffer, long size) {
    glHandler->dataToBuffer(buffer, size, nullptr, gl::GL_STREAM_READ);
}


heightMap * InverseDistanceWeightedFilter::apply(heightMap *map, bool generateOutput) {
    using namespace gl;

    std::cout << "Applying inverse distance weighted filter using OpenGL..." << std::endl << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    shaderPaths = std::vector<std::string>();
    shaderPaths.emplace_back("kernelIDW.glsl");
    shaderPaths.emplace_back("discretization.glsl");
    shaderPaths.emplace_back("horizontalTotalWeights.glsl");
    shaderPaths.emplace_back("totalWeights.glsl");
    shaderPaths.emplace_back("horizontalSumIDW.glsl");
    shaderPaths.emplace_back("sumIDW.glsl");
    shaderPaths.emplace_back("averageIDW.glsl");

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

    auto bufferSpecs = std::vector<std::vector<bufferSpecifications>>();
    bufferSpecs.reserve(shaderPaths.size());
    // kernel
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_KERNEL_BUFFER, long(sizeof(double) * kernelRadius)}});
    // discretization
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_CLOSING_MASK_BUFFER, long(sizeof(double) * pixelCount)}});
    // horizontalTotalWeights
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_HORIZONTAL_TOTAL_WEIGHT_BUFFER, long(sizeof(double) *  pixelCount)}});
    // totalWeights
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_TOTAL_WEIGHT_BUFFER, long(sizeof(double) * pixelCount)}});
    // horizontalSumIDW
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_HORIZONTAL_SUM_BUFFER, long(sizeof(double) * pixelCount)}});
    // sumIDW
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_SUM_BUFFER, long(sizeof(double) * pixelCount)}});
    // averageIDW
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_AVERAGE_BUFFER, long(sizeof(double) * pixelCount)}});
    // horizontalAmount
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_HORIZONTAL_AMOUNT_BUFFER, long(sizeof(unsigned) * pixelCount)}});
    // amount
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_AMOUNT_BUFFER, long(sizeof(unsigned) * pixelCount)}});
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

    for (auto i = 0ul; i < shader.size(); i++) {
        for (auto spec : bufferSpecs[i]) allocBuffer(spec.buffer, long(spec.size));

        glHandler->setProgram(shader[i]);
        glUniform2ui(glGetUniformLocation(shader[i], "resolution"), map->resolutionX, map->resolutionY);
        glUniform1ui(glGetUniformLocation(shader[i], "kernelRadius"), kernelRadius);
        glHandler->dispatchShader(batchSize, map->resolutionX, map->resolutionY);
        GLHandler::waitForShaderStorageIntegrity();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for closing: " << duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    if (!generateOutput) return emptyHeightMapfromHeightMap(map);

    auto filledMap = emptyHeightMapfromHeightMap(map);
    glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                              0,
                              (long) (sizeof(GLdouble) * filledMap->resolutionX * filledMap->resolutionY),
                              filledMap->heights.data());

    return filledMap;
}