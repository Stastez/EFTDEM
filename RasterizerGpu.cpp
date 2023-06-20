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

    auto workgroupSize = (unsigned int) (std::ceil(std::sqrt((double) pointGrid->numberOfPoints)));
    workgroupSize = std::ceil(workgroupSize / 8.);

    auto bufferMask = glHandler->getCoherentBufferMask();
    if (!(bufferMask[GLHandler::EFTDEM_RAW_POINT_BUFFER]
            && bufferMask[GLHandler::EFTDEM_RAW_POINT_INDEX_BUFFER]
            && bufferMask[GLHandler::EFTDEM_SORTED_POINT_COUNT_BUFFER])) {
        if (!(bufferMask[GLHandler::EFTDEM_RAW_POINT_BUFFER])) {
            auto rawPoints = new std::vector<GLdouble>();

            for (const auto& cell : pointGrid->points) {
                for (auto point : cell) {
                    rawPoints->emplace_back(point.x);
                    rawPoints->emplace_back(point.y);
                    rawPoints->emplace_back(point.z);
                }
            }

            glHandler->dataToBuffer(GLHandler::EFTDEM_RAW_POINT_BUFFER,
                                    (long long) sizeof(GLdouble) * pointGrid->numberOfPoints * 3,
                                    rawPoints->data(), GL_STATIC_DRAW);
        }

        if (!(bufferMask[GLHandler::EFTDEM_RAW_POINT_INDEX_BUFFER])) {
            auto indices = new std::vector<GLuint>(pointGrid->numberOfPoints);

            unsigned long long maxIndex = 0;
            for (auto i = 0; i < pointGrid->points.size(); i++) {
                for (auto j = maxIndex; j < pointGrid->points[i].size() + maxIndex; j++) {
                    indices->at(j) = i;
                }

                maxIndex += pointGrid->points[i].size();
            }

            glHandler->dataToBuffer(GLHandler::EFTDEM_RAW_POINT_INDEX_BUFFER,
                                    (long long) sizeof(GLuint) * pointGrid->numberOfPoints,
                                    indices->data(), GL_STATIC_DRAW);
        }

        if (!(bufferMask[GLHandler::EFTDEM_SORTED_POINT_COUNT_BUFFER])) {
            //countChunks.glsl
            auto counts = new GLuint[pointGrid->resolutionX * pointGrid->resolutionY];
            for (auto i = 0; i < pointGrid->resolutionX * pointGrid->resolutionY; i++) counts[i] = 0u;
            glHandler->dataToBuffer(GLHandler::EFTDEM_SORTED_POINT_COUNT_BUFFER,
                                    (long long) sizeof(GLuint) * pointGrid->resolutionX * pointGrid->resolutionY,
                                    counts, GL_STREAM_READ);
            glHandler->bindBuffer(GLHandler::EFTDEM_UNBIND);

            glUniform1ui(glGetUniformLocation(glHandler->getProgram(), "numberOfPoints"), pointGrid->numberOfPoints);

            glDispatchCompute(workgroupSize, workgroupSize, 1);
            glHandler->waitForShaderStorageIntegrity();
        }
    }

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
    glHandler->waitForShaderStorageIntegrity();

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
        return emptyHeightMapfromPointGrid(pointGrid);
    }

    heightMap map = emptyHeightMapfromPointGrid(pointGrid);

    glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER, 0,
                              (long long) sizeof(GLdouble) * pointGrid->resolutionX * pointGrid->resolutionY,
                              map.heights.data());

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for averaging: " << duration_cast<std::chrono::milliseconds>(end - start) << std::endl;

    return map;
}