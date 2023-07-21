#include "RadialFiller.h"
#include "FillerLoop.h"

#include <iostream>
#include <thread>

RadialFiller::RadialFiller(GLHandler * glHandler, unsigned int maxHoleRadius) {
    RadialFiller::glHandler = glHandler;
    RadialFiller::maxHoleRadius = maxHoleRadius;
    RadialFiller::stageUsesGPU = true;
}

heightMap *RadialFiller::apply(heightMap *map, bool generateOutput) {
    using namespace gl;

    std::cout << "Filling height map using radial filler..." << std::endl;

    auto begin = std::chrono::high_resolution_clock::now();

    // dilation
    auto dilationProgram = glHandler->getShaderPrograms({"radialDilation.glsl"}, true).at(0);
    auto dilationResolutionLocation = gl::glGetUniformLocation(dilationProgram, "resolution");
    auto dilationFlippedLocation = gl::glGetUniformLocation(dilationProgram, "flipped");

    glHandler->setProgram(dilationProgram);

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

    glUniform2ui(dilationResolutionLocation, map->resolutionX, map->resolutionY);

    GLsync previousSync = nullptr;
    for (auto i = 0u; i < maxHoleRadius; i++) {
        glUniform1i(dilationFlippedLocation, (bool) (i % 2u));
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

    // erosion
    auto erosionProgram = glHandler->getShaderPrograms({"radialErosion.glsl"}, true).at(0);
    auto erosionResolutionLocation = gl::glGetUniformLocation(erosionProgram, "resolution");
    auto erosionFlippedLocation = gl::glGetUniformLocation(erosionProgram, "flipped");

    glHandler->setProgram(erosionProgram);

    glUniform2ui(erosionResolutionLocation, map->resolutionX, map->resolutionY);

    previousSync = nullptr;
    for (auto i = 0u; i < maxHoleRadius; i++) {
        glUniform1i(erosionFlippedLocation, (bool) ((maxHoleRadius + i) % 2u));
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