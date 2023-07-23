#include "RadialFiller.h"
#include "FillerLoop.h"

#include <iostream>
#include <thread>
#include <cmath>

RadialFiller::RadialFiller(GLHandler * glHandler, unsigned int maxHoleRadius) {
    RadialFiller::glHandler = glHandler;
    RadialFiller::maxHoleRadius = maxHoleRadius;
    RadialFiller::stageUsesGPU = true;
}

RadialFiller::~RadialFiller() {
    glHandler->deleteBuffer(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER);
}

void dispatchCompute(gl::GLint flippedLocation, bool isDilation, unsigned int maxHoleRadius, heightMap * map) {
    using namespace gl;

    GLsync previousSync = nullptr;
    for (auto i = 0u; i < maxHoleRadius; i++) {
        auto flipped = (bool) (isDilation ? (i % 2u) : (maxHoleRadius + i) % 2u);
        glUniform1i(flippedLocation, flipped);
        glDispatchCompute((GLuint) std::ceil((double) map->resolutionX / 8.), (GLuint) std::ceil((double) map->resolutionY / 4.), 1);

        if (previousSync != nullptr) {
            glClientWaitSync(previousSync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
            glDeleteSync(previousSync);
        }

        auto currentSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        std::swap(previousSync, currentSync);
    }
    if (previousSync != nullptr) {
        glClientWaitSync(previousSync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
        glDeleteSync(previousSync);
    }
}

heightMap *RadialFiller::apply(heightMap *map, bool generateOutput) {
    using namespace gl;

    std::cout << "Filling height map using radial filler..." << std::endl;

    auto begin = std::chrono::high_resolution_clock::now();

    // dilation
    auto program = glHandler->getShaderPrograms({"radialDilation.glsl"}, true).at(0);
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
    }

    glUniform2ui(resolutionLocation, map->resolutionX, map->resolutionY);

    dispatchCompute(flippedLocation, true, maxHoleRadius, map);

    // erosion
    program = glHandler->getShaderPrograms({"radialErosion.glsl"}, true).at(0);
    resolutionLocation = gl::glGetUniformLocation(program, "resolution");
    flippedLocation = gl::glGetUniformLocation(program, "flipped");

    glHandler->setProgram(program);

    glUniform2ui(resolutionLocation, map->resolutionX, map->resolutionY);

    dispatchCompute(flippedLocation, false, maxHoleRadius, map);

    auto filledMap = emptyHeightMapFromHeightMap(map);

    if (maxHoleRadius % 2u == 1) {
        glHandler->dataFromBuffer(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER,
                                  0, map->dataSize, filledMap->heights.data());
        glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                map->dataSize,
                                filledMap->heights.data(), GL_STATIC_DRAW);
    } else {
        glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                  0, map->dataSize, filledMap->heights.data());
    }

    std::cout << "Elapsed time for radial filler: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin).count() << " ms" << std::endl;

    if (!generateOutput) {
        delete filledMap;
        return emptyHeightMapFromHeightMap(map);
    }

    return filledMap;
}