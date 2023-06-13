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

    auto shader = glHandler->getShaderPrograms({"../../shaders/countChunks.glsl", "../../shaders/sumChunks.glsl", "../../shaders/makeHeightmap.glsl"});
    glHandler->setProgram(shader[0]);

    //TODO
    if (!(glHandler->getCoherentBufferMask()[GLHandler::EFTDEM_RAW_POINT_BUFFER] && glHandler->getCoherentBufferMask()[GLHandler::EFTDEM_RAW_POINT_INDEX_BUFFER])) {

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
        glHandler->dataToBuffer(GLHandler::EFTDEM_RAW_POINT_INDEX_BUFFER,
                                (long long) sizeof(unsigned int) * pointGrid->resolutionX * pointGrid->resolutionY,
                                chunkBorders, GL_STATIC_DRAW);
        delete[] chunkBorders;
    }

    //countChunks.glsl
    auto counts = new GLuint[pointGrid->resolutionX * pointGrid->resolutionY];
    for (auto i = 0; i < pointGrid->resolutionX * pointGrid->resolutionY; i++) counts[i] = 0u;
    glHandler->dataToBuffer(GLHandler::EFTDEM_SORTED_POINT_COUNT_BUFFER,
                            (long long) sizeof(GLuint) * pointGrid->resolutionX * pointGrid->resolutionY,
                            counts, GL_STREAM_READ);
    glHandler->bindBuffer(GLHandler::EFTDEM_UNBIND);

    glUniform1ui(glGetUniformLocation(glHandler->getProgram(), "numberOfPoints"), pointGrid->numberOfPoints);

    auto workgroupSize = (unsigned int) (std::ceil(std::sqrt((double) pointGrid->numberOfPoints)));
    workgroupSize = std::ceil(workgroupSize / 8.);
    glDispatchCompute(workgroupSize, workgroupSize, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    //sumChunks.glsl
    glHandler->setProgram(shader[1]);
    auto sums = new GLuint[pointGrid->resolutionX * pointGrid->resolutionY];
    for (auto i = 0; i < pointGrid->resolutionX * pointGrid->resolutionY; i++) sums[i] = 0u;
    glHandler->dataToBuffer(GLHandler::EFTDEM_SORTED_POINT_SUM_BUFFER,
                            (long long) sizeof(GLuint) * pointGrid->resolutionX * pointGrid->resolutionY,
                            sums, GL_STREAM_READ);
    glHandler->bindBuffer(GLHandler::EFTDEM_UNBIND);

    glUniform2ui(glGetUniformLocation(glHandler->getProgram(), "resolution"), pointGrid->resolutionX, pointGrid->resolutionY);
    glUniform1ui(glGetUniformLocation(glHandler->getProgram(), "numberOfPoints"), pointGrid->numberOfPoints);

    glDispatchCompute(workgroupSize, workgroupSize, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    //makeHeightmap.glsl
    glHandler->setProgram(shader[2]);
    glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                            (long long) sizeof(GLdouble) * pointGrid->resolutionX * pointGrid->resolutionY,
                            nullptr, GL_STREAM_READ);
    glHandler->bindBuffer(GLHandler::EFTDEM_UNBIND);

    glUniform2ui(glGetUniformLocation(glHandler->getProgram(), "resolution"), pointGrid->resolutionX, pointGrid->resolutionY);

    glDispatchCompute(std::ceil(pointGrid->resolutionX / 8.), std::ceil(pointGrid->resolutionY / 8.), 1);

    if (!generateOutput) {
        glHandler->waitForShaderStorageIntegrity();
        return {};
    }

    heightMap map = emptyHeightMapfromPointGrid(pointGrid);

    glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER, 0,
                              (long long) sizeof(GLdouble) * pointGrid->resolutionX * pointGrid->resolutionY,
                              map.heights.data());

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for averaging: " << duration_cast<std::chrono::milliseconds>(end - start) << std::endl;

    return map;
}