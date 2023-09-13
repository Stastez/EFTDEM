#include "SorterGpu.h"
#include <iostream>
#include <cmath>

SorterGPU::SorterGPU(GLHandler *glHandler, unsigned long pixelPerUnitX, unsigned long pixelPerUnitY) {
    SorterGPU::stageUsesGPU = true;
    SorterGPU::glHandler = glHandler;
    SorterGPU::pixelPerUnitX = pixelPerUnitX;
    SorterGPU::pixelPerUnitY = pixelPerUnitY;
    SorterGPU::resolutionX = 0;
    SorterGPU::resolutionY = 0;
}

SorterGPU::~SorterGPU() = default;

/**
 * Creates a sorted grid of points from the points contained in the given rawPointCloud. The grid resolution is calculated
 * using the previously set pixelPerUnit{x,y}.
 * @param pointCloud The rawPointCloud to sort
 * @param generateOutput Whether to create a pointGrid. May be false if the next stage uses the same buffers
 * @return A fully formed pointGrid if generateOutput is true, a pointGrid only containing all necessary metadata otherwise
 */
pointGrid * SorterGPU::apply(rawPointCloud *pointCloud, bool generateOutput) {
    using namespace gl;

    std::cout << "Sorting points into grid using OpenGL..." << std::endl;

    std::vector<std::string> shaderPaths = {"chunkSortingAndCounting.glsl"};
    auto shaderPrograms = glHandler->getShaderPrograms(shaderPaths, true);
    glHandler->setProgram(shaderPrograms.at(0));

    auto points = new float[pointCloud->numberOfPoints * 3];

    unsigned int dataIndex = 0;
    doublePoint max = pointCloud->max, min = pointCloud->min;
    for (auto point : pointCloud->groundPoints) {
        points[dataIndex++] = normalizeValue(point.x, min.x, max.x);
        points[dataIndex++] = normalizeValue(point.y, min.y, max.y);
        points[dataIndex++] = normalizeValue(point.z, min.z, max.z);
    }

    glHandler->dataToBuffer(GLHandler::EFTDEM_RAW_POINT_BUFFER, (long long) (3 * pointCloud->numberOfPoints * sizeof(GLfloat)), points, GL_STATIC_DRAW);
    delete[] points;

    if (resolutionX == 0 || resolutionY == 0){
        resolutionX = std::max((unsigned long) std::ceil((std::abs(pointCloud->max.x - pointCloud->min.x)) * (double) pixelPerUnitX) + 1, 1ul);
        resolutionY = std::max((unsigned long) std::ceil((std::abs(pointCloud->max.y - pointCloud->min.y)) * (double) pixelPerUnitY) + 1, 1ul);
    }

    glUniform2ui(glGetUniformLocation(glHandler->getProgram(), "resolution"), resolutionX, resolutionY);
    glUniform1ui(glGetUniformLocation(glHandler->getProgram(), "numberOfPoints"), pointCloud->numberOfPoints);

    auto counts = new GLuint[resolutionX * resolutionY];

    for (unsigned long i = 0; i < resolutionX * resolutionY; i++) counts[i] = 0u;
    glHandler->dataToBuffer(GLHandler::EFTDEM_SORTED_POINT_COUNT_BUFFER,
                            (long) (sizeof(GLuint) * resolutionX * resolutionY),
                            counts, GL_STREAM_READ);

    delete[] counts;

    auto gridIndices = new std::vector<GLuint>(pointCloud->numberOfPoints, 0u);
    glHandler->dataToBuffer(GLHandler::EFTDEM_RAW_POINT_INDEX_BUFFER, (long long) (pointCloud->numberOfPoints * sizeof(GLuint)), gridIndices->data(), GL_STATIC_DRAW);
    glHandler->bindBuffer(GLHandler::EFTDEM_UNBIND);

    auto workgroupSize = (unsigned int) std::ceil(std::sqrt(pointCloud->numberOfPoints));
    glDispatchCompute((GLuint) std::ceil(workgroupSize / 8.), (GLuint) std::ceil(workgroupSize / 8.), 1);

    if (!generateOutput) {
        GLHandler::waitForShaderStorageIntegrity();
        delete gridIndices;
        return new pointGrid{.points = {},
                .resolutionX = resolutionX,
                .resolutionY = resolutionY,
                .min = min,
                .max = max,
                .numberOfPoints = pointCloud->numberOfPoints};
    }

    glHandler->dataFromBuffer(GLHandler::EFTDEM_RAW_POINT_INDEX_BUFFER, 0, (long long) (pointCloud->numberOfPoints * sizeof(GLuint)), gridIndices->data());

    auto grid = new pointGrid{.points = std::vector<std::vector<floatPoint>>(resolutionX * resolutionY),
                      .resolutionX = resolutionX,
                      .resolutionY = resolutionY,
                      .min = min,
                      .max = max,
                      .numberOfPoints = pointCloud->numberOfPoints};

    for (auto i = 0ul; i < gridIndices->size(); i++) {
        grid->points.at(gridIndices->at(i)).emplace_back(normalizeValue(pointCloud->groundPoints.at(i), min, max));
    }

    delete gridIndices;

    return grid;
}