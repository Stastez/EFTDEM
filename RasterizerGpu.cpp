#include <iostream>
#include "RasterizerGpu.h"

RasterizerGPU::RasterizerGPU(GLHandler *glHandler) {
    RasterizerGPU::glHandler = glHandler;
    RasterizerGPU::stageUsesGPU = true;
}

void RasterizerGPU::cleanUp() {

}

heightMap RasterizerGPU::apply(pointGrid *pointGrid, bool generateOutput) {
    using namespace gl;

    std::cout << "Rasterizing points to height map using OpenGL..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    if (!glHandler->isInitialized()) glHandler->initializeGL(false);
    std::vector<std::string> shaderPaths;
    shaderPaths.emplace_back("../../shaders/averageHeight.glsl");
    auto shader = glHandler->getShaderPrograms(shaderPaths);
    glHandler->setProgram(shader[0]);

    auto data = new double[pointGrid->numberOfPoints];
    auto chunkBorders = new unsigned int[pointGrid->resolutionX * pointGrid->resolutionY];
    unsigned int dataPosition = 0;
    for (auto i = 0; i < pointGrid->resolutionX * pointGrid->resolutionY; i++) {
        chunkBorders[i] = dataPosition;
        for (auto point: pointGrid->points[i]) {
            data[dataPosition++] = point.z;
        }
    }

    glHandler->dataToBuffer(GLHandler::EFTDEM_RAW_POINT_BUFFER,
                            (long long) sizeof(double) * pointGrid->numberOfPoints,
                            data, GL_STATIC_DRAW);
    delete[] data;
    glHandler->dataToBuffer(GLHandler::EFTDEM_RAW_POINT_COUNT_BUFFER,
                            (long long) sizeof(unsigned int) * pointGrid->resolutionX * pointGrid->resolutionY,
                            chunkBorders, GL_STATIC_DRAW);
    delete[] chunkBorders;
    glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                            (long long) sizeof(double) * pointGrid->resolutionX * pointGrid->resolutionY,
                            nullptr, GL_STREAM_READ);
    glHandler->bindBuffer(GLHandler::EFTDEM_UNBIND);

    glUniform2ui(glGetUniformLocation(shader[0], "resolution"), pointGrid->resolutionX, pointGrid->resolutionY);

    glDispatchCompute(pointGrid->resolutionX / 8, pointGrid->resolutionY / 8, 1);

    if (!generateOutput) {
        glHandler->waitForShaderStorageIntegrity();
        return {};
    }

    heightMap map = emptyHeightMapfromPointGrid(pointGrid);

    glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER, 0,
                              (long long) sizeof(double) * pointGrid->resolutionX * pointGrid->resolutionY,
                              map.heights.data());

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for averaging: " << duration_cast<std::chrono::milliseconds>(end - start) << std::endl;

    return map;
}