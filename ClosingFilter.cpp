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

/**
 * Deletes all buffers used for closing, except for the result buffer.
 */
ClosingFilter::~ClosingFilter() {
    glHandler->deleteBuffer(GLHandler::EFTDEM_CLOSING_MASK_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_HORIZONTAL_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_TOTAL_WEIGHT_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_SUM_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_AVERAGE_BUFFER);
}

/**
 * Applies the ClosingFilter
 * @param map the Heightmap that should be closed (if the Heightmap is already in the EFTDEM_HEIGHTMAP_BUFFER, the given map won't be used and may be an empty heightMap)
 * @param generateOutput specifies whether to return a heightMap Object with the filled map or just an empty heightMap. This improves efficiency, by only moving Data from and to graphics memory when necessary.
 * @return a heightMap with the filled height-data, or an empty map.
 */
heightMap * ClosingFilter::apply(heightMap *map, bool generateOutput) {
    using namespace gl;

    std::cout << "Applying closing filter using OpenGL..." << std::endl << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    //The shaders in this vector will be executed
    shaderPaths = std::vector<std::string>();

    auto pixelCount = (long) (map->resolutionX * map->resolutionY);

    /*This vector should contain a vector of bufferSpecifications for every Shader specified in shaderPaths,
     * containing the Specifications for all Buffers that need to be initialized before the respective shader is executed.*/
    auto bufferSpecs = std::vector<std::vector<bufferSpecifications>>();

    shaderPaths.emplace_back("discretization.glsl");
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_CLOSING_MASK_BUFFER, sizeof(GLfloat)}});
    shaderPaths.emplace_back("horizontalAmount.glsl");
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_HORIZONTAL_BUFFER, sizeof(GLfloat)}});
    shaderPaths.emplace_back("amount.glsl");
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_TOTAL_WEIGHT_BUFFER, long(sizeof(unsigned int) * pixelCount)}});
    shaderPaths.emplace_back("horizontalSum.glsl");
    bufferSpecs.emplace_back();
    shaderPaths.emplace_back("sum.glsl");
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_SUM_BUFFER, sizeof(GLfloat)}});
    shaderPaths.emplace_back("average.glsl");
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_AVERAGE_BUFFER, sizeof(GLfloat)}});

    shaderPaths.emplace_back("dilation.glsl");
    bufferSpecs.emplace_back();
    shaderPaths.emplace_back("horizontalAmount.glsl");
    bufferSpecs.emplace_back();
    shaderPaths.emplace_back("amount.glsl");
    bufferSpecs.emplace_back();
    shaderPaths.emplace_back("erosion.glsl");
    bufferSpecs.emplace_back();

    shaderPaths.emplace_back("closing.glsl");
    bufferSpecs.emplace_back();


    auto shader = glHandler->getShaderPrograms(shaderPaths, true);

    //if the height-data is already in the EFTDEM_HEIGHTMAP_BUFFER the coherent-buffer-mask will indicate that and the data does not need to be put on the Buffer
    if (!glHandler->getCoherentBufferMask().at(GLHandler::EFTDEM_HEIGHTMAP_BUFFER)){
        glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                (long) (sizeof(GLfloat) * pixelCount),
                                map->heights.data(), GL_STATIC_DRAW);
    }

    //TODO remove?
    glHandler->setProgram(shader.at(0));

    for (auto i = 0ul; i < bufferSpecs.size(); i++) {
        for (auto spec : bufferSpecs.at(i)) allocBuffer(spec.buffer, long(spec.size) * pixelCount);

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