#include "GradientBasedFiller.h"
#include <iostream>

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
};

heightMap * GradientBasedFiller::apply(heightMap *map, bool generateOutput) {
    using namespace gl;

    std::cout << "Applying gradient based filter using OpenGL..." << std::endl << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    shaderPaths = std::vector<std::string>();
    shaderPaths.emplace_back("gradient.glsl");
    shaderPaths.emplace_back("kernelIDW.glsl");
    shaderPaths.emplace_back("gradientHorizontalTotalWeight.glsl");
    shaderPaths.emplace_back("gradientTotalWeights.glsl");
    shaderPaths.emplace_back("gradientHorizontalSum.glsl");
    shaderPaths.emplace_back("gradientSum.glsl");
    shaderPaths.emplace_back("gradientFiller.glsl");

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
    // gradient
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_GRADIENT_BUFFER, long(sizeof(GLfloat) * pixelCount * 2)}});
    // kernel
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_KERNEL_BUFFER, long(sizeof(GLfloat) * kernelRadius)}});
    // gradientHorizontalTotalWeights
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_HORIZONTAL_BUFFER, long(sizeof(GLfloat) * pixelCount * 2)}});
    // totalWeights
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_TOTAL_WEIGHT_BUFFER, long(sizeof(GLfloat) * pixelCount * 2)}});
    // horizontalSum
    bufferSpecs.emplace_back();
    // sum
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_SUM_BUFFER, long(sizeof(GLfloat) * pixelCount * 2)}});
    // averageIDW
    //bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_AVERAGE_BUFFER, long(sizeof(GLfloat) * pixelCount)}});
    // gradientFiller
    bufferSpecs.emplace_back();

    for (auto i = 0ul; i < shader.size(); i++) {
        for (auto spec : bufferSpecs.at(i)) allocBuffer(spec.buffer, long(spec.size));

        glHandler->setProgram(shader.at(i));
        glUniform2ui(glGetUniformLocation(shader.at(i), "resolution"), map->resolutionX, map->resolutionY);
        glUniform1ui(glGetUniformLocation(shader.at(i), "kernelRadius"), kernelRadius);
        glHandler->dispatchShader(batchSize, map->resolutionX, map->resolutionY);
        GLHandler::waitForShaderStorageIntegrity();
    }

    // dilation
    /*auto program = glHandler->getShaderPrograms({"radialDilation.glsl"}, true).at(0);
    auto resolutionLocation = gl::glGetUniformLocation(program, "resolution");
    auto flippedLocation = gl::glGetUniformLocation(program, "flipped");

    glHandler->setProgram(program);

    if (!glHandler->getCoherentBufferMask().at(GLHandler::EFTDEM_HEIGHTMAP_BUFFER)){
        glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                map->dataSize,
                                map->heights.data(), GL_STATIC_DRAW);
    }

    if (!glHandler->getCoherentBufferMask().at(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER)) {
        auto initialState = new std::vector<GLfloat>(map->resolutionX * map->resolutionY, 0);
        glHandler->dataToBuffer(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER,
                                map->dataSize,
                                initialState->data(), GL_STATIC_DRAW);
        delete initialState;
    }

    glUniform2ui(resolutionLocation, map->resolutionX, map->resolutionY);

    dispatchCompute(flippedLocation, true, map);

    // erosion
    program = glHandler->getShaderPrograms({"radialErosion.glsl"}, true).at(0);
    resolutionLocation = gl::glGetUniformLocation(program, "resolution");
    flippedLocation = gl::glGetUniformLocation(program, "flipped");

    glHandler->setProgram(program);

    glUniform2ui(resolutionLocation, map->resolutionX, map->resolutionY);

    dispatchCompute(flippedLocation, false, map);

    // Actual radial filler output
    auto filledMap = emptyHeightMapFromHeightMap(map);

    if (kernelRadius % 2u == 1) {
        glHandler->dataFromBuffer(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER,
                                  0, map->dataSize, filledMap->heights.data());
        glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                map->dataSize,
                                filledMap->heights.data(), GL_STATIC_DRAW);
    } else {
        glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                  0, map->dataSize, filledMap->heights.data());
    }*/

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
