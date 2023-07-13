#include "RadialEroder.h"

#include <iostream>
#include <cmath>

RadialEroder::RadialEroder(GLHandler *glHandler, bool flipped, unsigned int batchSize, bool batched,
                           gl::GLuint shaderProgram, gl::GLint resolutionLocation, gl::GLint currentInvocationLocation,
                           gl::GLint flippedLocation, bool ignoreOutput) {
    RadialEroder::glHandler = glHandler;
    RadialEroder::stageUsesGPU = true;
    RadialEroder::flipped = flipped;
    RadialEroder::batchSize = batchSize;
    RadialEroder::batched = batched;
    RadialEroder::shaderProgram = shaderProgram;
    RadialEroder::resolutionLocation = resolutionLocation;
    RadialEroder::currentInvocationLocation = currentInvocationLocation;
    RadialEroder::flippedLocation = flippedLocation;
    RadialEroder::ignoreOutput = ignoreOutput;
    RadialEroder::stageUsesGPU = true;
}

heightMap *RadialEroder::apply(heightMap *map, bool generateOutput) {
    using namespace gl;

    /*auto shaderPaths = new std::vector<std::string>{"radialErosion.glsl"};

    auto shaders = glHandler->getShaderPrograms(*shaderPaths, true);*/

    glHandler->setProgram(shaderProgram);
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
    glUniform2ui(resolutionLocation, map->resolutionX, map->resolutionY);
    glUniform1i(flippedLocation, flipped);
    if (batched)
        glHandler->dispatchShader(batchSize, map->resolutionX, map->resolutionY);
    else {
        glUniform2ui(currentInvocationLocation, 0, 0);
        glDispatchCompute((GLuint) std::ceil((double) map->resolutionX / 8.), (GLuint) std::ceil((double) map->resolutionY / 4.), 1);
    }
    GLHandler::waitForShaderStorageIntegrity();

    if (ignoreOutput || !generateOutput) return emptyHeightMapFromHeightMap(map);

    auto filledMap = emptyHeightMapFromHeightMap(map);

    if (flipped) glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                     0, map->dataSize, filledMap->heights.data());
    else glHandler->dataFromBuffer(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER,
                                   0, map->dataSize, filledMap->heights.data());

    return filledMap;
}
