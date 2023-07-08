#include "RadialDilator.h"

#include <iostream>
#include <cmath>

RadialDilator::RadialDilator(GLHandler *glHandler, bool flipped, unsigned int batchSize) {
    RadialDilator::glHandler = glHandler;
    RadialDilator::stageUsesGPU = true;
    RadialDilator::flipped = flipped;
    RadialDilator::batchSize = batchSize;
}

heightMap *RadialDilator::apply(heightMap *map, bool generateOutput) {
    using namespace gl;

    auto shaderPaths = new std::vector<std::string>{"radialDilation.glsl"};

    auto shaders = glHandler->getShaderPrograms(*shaderPaths, true);

    glHandler->setProgram(shaders.at(0));

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
    }

    // radialDilation.glsl
    glUniform2ui(glGetUniformLocation(glHandler->getProgram(), "resolution"), map->resolutionX, map->resolutionY);
    glUniform1i(glGetUniformLocation(glHandler->getProgram(), "flipped"), flipped);
    glHandler->dispatchShader(batchSize, map->resolutionX, map->resolutionY);
    //glUniform2ui(glGetUniformLocation(glHandler->getProgram(), "currentInvocation"), 0, 0);
    //glDispatchCompute((GLuint) std::ceil((double) map->resolutionX / 8.), (GLuint) std::ceil((double) map->resolutionY / 4.), 1);
    GLHandler::waitForShaderStorageIntegrity();

    if (!generateOutput) return emptyHeightMapFromHeightMap(map);

    auto filledMap = emptyHeightMapFromHeightMap(map);

    glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                              0, map->dataSize, filledMap->heights.data());

    return filledMap;
}
