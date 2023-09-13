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

/**
 * Applies radial filling to the heightMap passed in map. map may be empty if a previous pipeline stage has already
 * written the height map to EFTDEM_HEIGHTMAP_BUFFER.
 * @param map The heightMap to fill or an empty heightMap
 * @param generateOutput Whether or not to return a fully formed heightMap. This may be set false if the stage immediately
 *                      after uses GPU buffers
 * @return A filled heightMap if generateOutput is true, a heightMap only containing all necessary metadata otherwise
 */
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
        delete initialState;
    }

    glUniform2ui(resolutionLocation, map->resolutionX, map->resolutionY);

    glHandler->dispatchShader(flippedLocation, true, map->resolutionX, map->resolutionY, maxHoleRadius);

    // erosion
    program = glHandler->getShaderPrograms({"radialErosion.glsl"}, true).at(0);
    resolutionLocation = gl::glGetUniformLocation(program, "resolution");
    flippedLocation = gl::glGetUniformLocation(program, "flipped");

    glHandler->setProgram(program);

    glUniform2ui(resolutionLocation, map->resolutionX, map->resolutionY);

    glHandler->dispatchShader(flippedLocation, false, map->resolutionX, map->resolutionY, maxHoleRadius);

    auto filledMap = emptyHeightMapFromHeightMap(map);
    glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                              0, map->dataSize, filledMap->heights.data());

    std::cout << "Elapsed time for radial filler: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - begin).count() << " ms" << std::endl;

    if (!generateOutput) {
        delete filledMap;
        return emptyHeightMapFromHeightMap(map);
    }

    return filledMap;
}