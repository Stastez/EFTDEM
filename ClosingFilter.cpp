#include "ClosingFilter.h"
#include "DataStructures.h"
#include "GLHandler.h"
#include <iostream>

ClosingFilter::ClosingFilter(GLHandler *glHandler, unsigned int kernelRadius, unsigned int batchSize = 0) {
    ClosingFilter::glHandler = glHandler;
    ClosingFilter::kernelRadius = kernelRadius;
    ClosingFilter::batchSize = batchSize;
    ClosingFilter::stageUsesGPU = true;
}

ClosingFilter::~ClosingFilter() {
    auto bufferMask = glHandler->getCoherentBufferMask();

    glHandler->deleteBuffer(GLHandler::EFTDEM_CLOSING_MASK_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_HORIZONTAL_AMOUNT_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_AMOUNT_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_HORIZONTAL_SUM_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_SUM_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_AVERAGE_BUFFER);
};

void ClosingFilter::allocBuffer(GLHandler::bufferIndices buffer, long singleDataSize, long dataCount) {
    glHandler->dataToBuffer(buffer, singleDataSize * dataCount, nullptr, gl::GL_STATIC_DRAW);
}

heightMap * ClosingFilter::apply(heightMap *map, bool generateOutput) {
    using namespace gl;

    std::cout << "Applying closing filter using OpenGL..." << std::endl << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    shaderPaths = std::vector<std::string>();
    shaderPaths.emplace_back("discretization.glsl");
    shaderPaths.emplace_back("horizontalAmount.glsl");
    shaderPaths.emplace_back("amount.glsl");
    shaderPaths.emplace_back("horizontalSum.glsl");
    shaderPaths.emplace_back("sum.glsl");
    shaderPaths.emplace_back("average.glsl");

    shaderPaths.emplace_back("dilation.glsl");
    shaderPaths.emplace_back("horizontalAmount.glsl");
    shaderPaths.emplace_back("amount.glsl");
    shaderPaths.emplace_back("erosion.glsl");

    shaderPaths.emplace_back("closing.glsl");

    auto shader = glHandler->getShaderPrograms(shaderPaths, true);

    auto pixelCount = (long) (map->resolutionX * map->resolutionY);

    glHandler->setProgram(shader.at(0));

    if (!glHandler->getCoherentBufferMask().at(GLHandler::EFTDEM_HEIGHTMAP_BUFFER)){
        glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                (long) (sizeof(GLfloat) * pixelCount),
                                map->heights.data(), GL_STATIC_DRAW);
    }

    auto bufferSpecs = std::vector<std::vector<bufferSpecifications>>();
    bufferSpecs.reserve(shaderPaths.size());
    // discretization
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_CLOSING_MASK_BUFFER, sizeof(GLfloat)}});
    // horizontalAmount
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_HORIZONTAL_AMOUNT_BUFFER, sizeof(unsigned)}});
    // amount
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_AMOUNT_BUFFER, sizeof(unsigned)}});
    // horizontalSum
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_HORIZONTAL_SUM_BUFFER, sizeof(GLfloat)}});
    // sum
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_SUM_BUFFER, sizeof(GLfloat)}});
    // average
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_AVERAGE_BUFFER, sizeof(GLfloat)}});
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
        for (auto spec : bufferSpecs.at(i)) allocBuffer(spec.buffer, long(spec.size), pixelCount);

        glHandler->setProgram(shader.at(i));
        glUniform2ui(glGetUniformLocation(glHandler->getProgram(), "resolution"), map->resolutionX, map->resolutionY);
        glUniform1ui(glGetUniformLocation(glHandler->getProgram(), "kernelRadius"), kernelRadius);
        glHandler->dispatchShader(batchSize, map->resolutionX, map->resolutionY);
        GLHandler::waitForShaderStorageIntegrity();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for closing: " << duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    if (!generateOutput) return emptyHeightMapFromHeightMap(map);

    auto filledMap = emptyHeightMapFromHeightMap(map);
    glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                              0,
                              (long) (sizeof(GLfloat) * filledMap->resolutionX * filledMap->resolutionY),
                              filledMap->heights.data());

    return filledMap;
}