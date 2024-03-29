#include "RasterizerGpu.h"
#include <iostream>
#include <cmath>

RasterizerGPU::RasterizerGPU(GLHandler *glHandler) {
    RasterizerGPU::glHandler = glHandler;
    RasterizerGPU::stageUsesGPU = true;
}

RasterizerGPU::~RasterizerGPU() {
    auto bufferCoherency = glHandler->getCoherentBufferMask();

    glHandler->deleteBuffer(GLHandler::EFTDEM_RAW_POINT_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_RAW_POINT_INDEX_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_SORTED_POINT_SUM_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_SORTED_POINT_COUNT_BUFFER);
}

/**
 * Creates a heightmap from the given point grid by averaging the points of every grid cell.
 * @param pointGrid The point grid containing the point data, sorted into a grid
 * @param generateOutput Whether or not to create output. May be false if the next stage uses the same GPU buffer
 * @return A new heightMap struct
 */
heightMap * RasterizerGPU::apply(pointGrid *pointGrid, bool generateOutput) {
    using namespace gl;

    std::cout << "Rasterizing points to height map using OpenGL..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    auto shader = glHandler->getShaderPrograms({"countChunks.glsl", "sumChunks.glsl", "makeHeightmap.glsl"}, true);
    glHandler->setProgram(shader.at(0));

    auto workgroupSize = (unsigned int) (std::ceil(std::sqrt((double) pointGrid->numberOfPoints)));
    workgroupSize = (unsigned int) std::ceil(workgroupSize / 8.);

    auto bufferMask = glHandler->getCoherentBufferMask();
    if (!(bufferMask.at(GLHandler::EFTDEM_RAW_POINT_BUFFER)
            && bufferMask.at(GLHandler::EFTDEM_RAW_POINT_INDEX_BUFFER)
            && bufferMask.at(GLHandler::EFTDEM_SORTED_POINT_COUNT_BUFFER))) {
        if (!(bufferMask.at(GLHandler::EFTDEM_RAW_POINT_BUFFER))) {
            auto rawPoints = new std::vector<GLfloat>();

            for (const auto& cell : pointGrid->points) {
                for (auto point : cell) {
                    rawPoints->emplace_back(point.x);
                    rawPoints->emplace_back(point.y);
                    rawPoints->emplace_back(point.z);
                }
            }

            glHandler->dataToBuffer(GLHandler::EFTDEM_RAW_POINT_BUFFER,
                                    (long) (sizeof(GLfloat) * pointGrid->numberOfPoints * 3),
                                    rawPoints->data(), GL_STATIC_DRAW);

            delete rawPoints;
        }

        if (!(bufferMask.at(GLHandler::EFTDEM_RAW_POINT_INDEX_BUFFER))) {
            auto indices = new std::vector<GLuint>(pointGrid->numberOfPoints);

            unsigned long long maxIndex = 0;
            for (auto i = 0ul; i < pointGrid->points.size(); i++) {
                for (auto j = maxIndex; j < pointGrid->points.at(i).size() + maxIndex; j++) {
                    indices->at(j) = (GLuint) i;
                }

                maxIndex += pointGrid->points.at(i).size();
            }

            glHandler->dataToBuffer(GLHandler::EFTDEM_RAW_POINT_INDEX_BUFFER,
                                    (long) (sizeof(GLuint) * pointGrid->numberOfPoints),
                                    indices->data(), GL_STATIC_DRAW);

            delete indices;
        }

        if (!(bufferMask.at(GLHandler::EFTDEM_SORTED_POINT_COUNT_BUFFER))) {
            //countChunks.glsl
            auto counts = new GLuint[pointGrid->resolutionX * pointGrid->resolutionY];
            for (unsigned long i = 0; i < pointGrid->resolutionX * pointGrid->resolutionY; i++) counts[i] = 0u;
            glHandler->dataToBuffer(GLHandler::EFTDEM_SORTED_POINT_COUNT_BUFFER,
                                    (long) (sizeof(GLuint) * pointGrid->resolutionX * pointGrid->resolutionY),
                                    counts, GL_STATIC_DRAW);
            glHandler->bindBuffer(GLHandler::EFTDEM_UNBIND);

            glUniform1ui(glGetUniformLocation(glHandler->getProgram(), "numberOfPoints"), pointGrid->numberOfPoints);

            glDispatchCompute(workgroupSize, workgroupSize, 1);

            delete[] counts;

            GLHandler::waitForShaderStorageIntegrity();
        }
    }

    //sumChunks.glsl
    glHandler->setProgram(shader.at(1));
    auto sums = new GLuint[pointGrid->resolutionX * pointGrid->resolutionY];
    for (unsigned long i = 0; i < pointGrid->resolutionX * pointGrid->resolutionY; i++) sums[i] = 0u;
    glHandler->dataToBuffer(GLHandler::EFTDEM_SORTED_POINT_SUM_BUFFER,
                            (long) (sizeof(GLuint) * pointGrid->resolutionX * pointGrid->resolutionY),
                            sums, GL_STATIC_DRAW);
    glHandler->bindBuffer(GLHandler::EFTDEM_UNBIND);

    glUniform1ui(glGetUniformLocation(glHandler->getProgram(), "numberOfPoints"), pointGrid->numberOfPoints);

    glDispatchCompute(workgroupSize, workgroupSize, 1);

    delete[] sums;

    GLHandler::waitForShaderStorageIntegrity();

    //makeHeightmap.glsl
    glHandler->setProgram(shader.at(2));
    glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                            (long) (sizeof(GLfloat) * pointGrid->resolutionX * pointGrid->resolutionY),
                            nullptr, GL_DYNAMIC_DRAW);
    glHandler->bindBuffer(GLHandler::EFTDEM_UNBIND);

    glUniform2ui(glGetUniformLocation(glHandler->getProgram(), "resolution"), pointGrid->resolutionX, pointGrid->resolutionY);

    glDispatchCompute((GLuint) std::ceil((double) pointGrid->resolutionX / 8.), (GLuint) std::ceil((double) pointGrid->resolutionY / 8.), 1);

    if (!generateOutput) {
        GLHandler::waitForShaderStorageIntegrity();
        return emptyHeightMapFromPointGrid(pointGrid);
    }

    auto map = emptyHeightMapFromPointGrid(pointGrid);

    glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER, 0,
                              (long) (sizeof(float) * pointGrid->resolutionX * pointGrid->resolutionY),
                              map->heights.data());

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for averaging: " << duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    return map;
}