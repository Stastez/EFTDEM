#include "GradientBasedFiller.h"
#include <iostream>
#include <cmath>

GradientBasedFiller::GradientBasedFiller(GLHandler *glHandler, unsigned int kernelRadius, unsigned int batchSize = 0) {
    GradientBasedFiller::glHandler = glHandler;
    GradientBasedFiller::kernelRadius = kernelRadius;
    GradientBasedFiller::batchSize = batchSize;
    GradientBasedFiller::stageUsesGPU = true;
}

GradientBasedFiller::~GradientBasedFiller() {
    //TODO: delete used buffers
    /*glHandler->deleteBuffer(GLHandler::EFTDEM_CLOSING_MASK_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_HORIZONTAL_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_TOTAL_WEIGHT_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_SUM_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_AVERAGE_BUFFER);*/
}

/**
 * Applies gradient-based filling on the provided heightMap.
 * @param map the Heightmap that should be closed (if the Heightmap is already in the EFTDEM_HEIGHTMAP_BUFFER, the given map won't be used and may be an empty heightMap)
 * @param generateOutput specifies whether to return a heightMap Object with the filled map or just an empty heightMap. This improves efficiency, by only moving Data from and to graphics memory when necessary.
 * @return a heightMap with the filled height-data, or an empty map.
 */
heightMap * GradientBasedFiller::apply(heightMap *map, bool generateOutput) {
    using namespace gl;

    std::cout << "Applying gradient based filter using OpenGL..." << std::endl << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    //The shaders in this vector will be executed
    shaderPaths = std::vector<std::string>();

    auto pixelCount = (long) (map->resolutionX * map->resolutionY);

    /*This vector should contain a vector of bufferSpecifications for every Shader specified in shaderPaths,
     * containing the Specifications for all Buffers that need to be initialized before the respective shader is executed.*/
    auto bufferSpecs = std::vector<std::vector<bufferSpecifications>>();

    shaderPaths.emplace_back("gradient.glsl");
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_GRADIENT_BUFFER, long(sizeof(GLfloat) * pixelCount * 2)}});
    shaderPaths.emplace_back("kernelIDW.glsl");
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_KERNEL_BUFFER, long(sizeof(GLfloat) * kernelRadius)}});
    shaderPaths.emplace_back("gradientTotalWeightHorizontal.glsl");
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_HORIZONTAL_BUFFER, long(sizeof(GLfloat) * pixelCount * 2)}});
    shaderPaths.emplace_back("gradientTotalWeights.glsl");
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_TOTAL_WEIGHT_BUFFER, long(sizeof(GLfloat) * pixelCount * 2)}});
    shaderPaths.emplace_back("gradientSumHorizontal.glsl");
    bufferSpecs.emplace_back();
    shaderPaths.emplace_back("gradientSum.glsl");
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_SUM_BUFFER, long(sizeof(GLfloat) * pixelCount * 2)}});
    shaderPaths.emplace_back("gradientFiller.glsl");
    bufferSpecs.emplace_back();

    shaderPaths.emplace_back("discretization.glsl");
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_CLOSING_MASK_BUFFER, long(sizeof(GLfloat) * pixelCount)}});
    shaderPaths.emplace_back("IDWsumHorizontal.glsl");
    bufferSpecs.emplace_back();
    shaderPaths.emplace_back("IDWsum.glsl");
    bufferSpecs.emplace_back();
    shaderPaths.emplace_back("totalWeightsHorizontal.glsl");
    bufferSpecs.emplace_back();
    shaderPaths.emplace_back("totalWeights.glsl");
    bufferSpecs.emplace_back();


    auto shader = glHandler->getShaderPrograms(shaderPaths, true);

    glHandler->setProgram(shader.at(0));

    //if the height-data is already in the EFTDEM_HEIGHTMAP_BUFFER the coherent-buffer-mask will indicate that and the data does not need to be put on the Buffer
    if (!glHandler->getCoherentBufferMask().at(GLHandler::EFTDEM_HEIGHTMAP_BUFFER)){
        glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                (long) (sizeof(GLfloat) * pixelCount),
                                map->heights.data(), GL_STATIC_DRAW);
    }

    for (auto i = 0ul; i < shader.size(); i++) {
        for (auto spec: bufferSpecs.at(i)) allocBuffer(spec.buffer, long(spec.size));

        glHandler->setProgram(shader.at(i));
        glUniform2ui(glGetUniformLocation(shader.at(i), "resolution"), map->resolutionX, map->resolutionY);
        glUniform1ui(glGetUniformLocation(shader.at(i), "kernelRadius"), kernelRadius);
        glHandler->dispatchShader(batchSize, map->resolutionX, map->resolutionY);
        GLHandler::waitForShaderStorageIntegrity();
    }

    // dilation
    auto program = glHandler->getShaderPrograms({"gradientRadialDilation.glsl"}, true).at(0);
    auto resolutionLocation = gl::glGetUniformLocation(program, "resolution");
    auto flippedLocation = gl::glGetUniformLocation(program, "flipped");

    glHandler->setProgram(program);

    if (!glHandler->getCoherentBufferMask().at(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER)) {
        auto initialState = new std::vector<GLfloat>(map->resolutionX * map->resolutionY, 0);
        glHandler->dataToBuffer(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER,
                                map->dataSize,
                                initialState->data(), GL_STATIC_DRAW);
        delete initialState;
    }

    glUniform2ui(resolutionLocation, map->resolutionX, map->resolutionY);

    glHandler->dispatchShader(flippedLocation, true, map->resolutionX, map->resolutionY, kernelRadius);

    // erosion
    program = glHandler->getShaderPrograms({"radialErosion.glsl"}, true).at(0);
    resolutionLocation = gl::glGetUniformLocation(program, "resolution");
    flippedLocation = gl::glGetUniformLocation(program, "flipped");

    glHandler->setProgram(program);

    glUniform2ui(resolutionLocation, map->resolutionX, map->resolutionY);

    glHandler->dispatchShader(flippedLocation, false, map->resolutionX, map->resolutionY, kernelRadius);

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for closing: " << duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    if (!generateOutput) return emptyHeightMapFromHeightMap(map);
    auto filledMap = emptyHeightMapFromHeightMap(map);

    glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                              0, map->dataSize, filledMap->heights.data());

    return filledMap;
}
