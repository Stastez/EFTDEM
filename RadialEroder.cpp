#include "RadialEroder.h"

#include <iostream>
#include <cmath>

RadialEroder::RadialEroder(GLHandler *glHandler, bool flipped, unsigned int batchSize, bool batched) {
    RadialEroder::glHandler = glHandler;
    RadialEroder::stageUsesGPU = true;
    RadialEroder::flipped = flipped;
    RadialEroder::batchSize = batchSize;
    RadialEroder::batched = batched;
}

heightMap *RadialEroder::apply(heightMap *map, bool generateOutput) {
    using namespace gl;

    auto shaderPaths = new std::vector<std::string>{"radialErosion.glsl"};

    auto shaders = glHandler->getShaderPrograms(*shaderPaths, true);

    glHandler->setProgram(shaders.at(0));

    if (!glHandler->getCoherentBufferMask().at(GLHandler::EFTDEM_HEIGHTMAP_BUFFER)){
        if (flipped) {
            auto initialState = new std::vector<GLfloat>(map->resolutionX * map->resolutionY, 0);
            glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                    map->dataSize,
                                    initialState->data(), GL_STATIC_DRAW);
        }
        else glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                map->dataSize,
                                map->heights.data(), GL_STATIC_DRAW);
    }

    if (!glHandler->getCoherentBufferMask().at(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER)) {
        if (!flipped) {
            auto initialState = new std::vector<GLfloat>(map->resolutionX * map->resolutionY, 0);
            glHandler->dataToBuffer(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER,
                                    map->dataSize,
                                    initialState->data(), GL_STATIC_DRAW);
        }
        else glHandler->dataToBuffer(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER,
                                     map->dataSize,
                                     map->heights.data(), GL_STATIC_DRAW);
    }

    // radialErosion.glsl
    glUniform2ui(glGetUniformLocation(glHandler->getProgram(), "resolution"), map->resolutionX, map->resolutionY);
    glUniform1i(glGetUniformLocation(glHandler->getProgram(), "flipped"), flipped);
    if (batched)
        glHandler->dispatchShader(batchSize, map->resolutionX, map->resolutionY);
    else {
        glUniform2ui(glGetUniformLocation(glHandler->getProgram(), "currentInvocation"), 0, 0);
        glDispatchCompute((GLuint) std::ceil((double) map->resolutionX / 8.), (GLuint) std::ceil((double) map->resolutionY / 4.), 1);
    }
    GLHandler::waitForShaderStorageIntegrity();

    if (!generateOutput) return emptyHeightMapFromHeightMap(map);

    auto filledMap = emptyHeightMapFromHeightMap(map);

    if (flipped) glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                     0, map->dataSize, filledMap->heights.data());
    else glHandler->dataFromBuffer(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER,
                                   0, map->dataSize, filledMap->heights.data());

    return filledMap;
}
