#include "RadialBufferSwapper.h"
#include <cmath>

RadialBufferSwapper::RadialBufferSwapper(GLHandler *glHandler) {
    RadialBufferSwapper::glHandler = glHandler;
    RadialBufferSwapper::stageUsesGPU = true;
}

heightMap *RadialBufferSwapper::apply(heightMap *map, bool generateOutput) {
    using namespace gl;

    auto filledMap = emptyHeightMapFromHeightMap(map);
    glHandler->dataFromBuffer(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER,
                              0, map->dataSize, filledMap->heights.data());
    glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                            map->dataSize,
                            filledMap->heights.data(), GL_STATIC_DRAW);

    if (!generateOutput) {
        delete filledMap;
        return emptyHeightMapFromHeightMap(map);
    }

    return filledMap;
}
