#include "SorterGpu.h"
#include <iostream>
#include <cmath>

SorterGPU::SorterGPU(GLHandler *glHandler, unsigned long pixelPerUnitX, unsigned long pixelPerUnitY) {
    SorterGPU::stageUsesGPU = true;
    SorterGPU::glHandler = glHandler;
    SorterGPU::pixelPerUnitX = pixelPerUnitX;
    SorterGPU::pixelPerUnitY = pixelPerUnitY;
}

SorterGPU::~SorterGPU() = default;

pointGrid * SorterGPU::apply(rawPointCloud *pointCloud, bool generateOutput) {
    using namespace gl;

    std::cout << "Sorting points into grid using OpenGL..." << std::endl;

    std::vector<std::string> shaderPaths = {"chunkSortingAndCounting.glsl"};
    auto shaderPrograms = glHandler->getShaderPrograms(shaderPaths, true);
    glHandler->setProgram(shaderPrograms.at(0));

    auto points = new double[pointCloud->numberOfPoints * 3];

    unsigned int dataIndex = 0;
    point max = pointCloud->max, min = pointCloud->min;
    for (auto point : pointCloud->groundPoints) {
        points[dataIndex++] = normalizeValue(point.x, min.x, max.x);
        points[dataIndex++] = normalizeValue(point.y, min.y, max.y);
        points[dataIndex++] = normalizeValue(point.z, min.z, max.z);
    }

    glHandler->dataToBuffer(GLHandler::EFTDEM_RAW_POINT_BUFFER, (long long) (3 * pointCloud->numberOfPoints * sizeof(GLdouble)), points, GL_STATIC_DRAW);
    delete[] points;

    unsigned long resolutionX = std::max((unsigned long) std::ceil((std::abs(pointCloud->max.x - pointCloud->min.x)) * (double) pixelPerUnitX) + 1, 1ul);
    unsigned long resolutionY = std::max((unsigned long) std::ceil((std::abs(pointCloud->max.y - pointCloud->min.y)) * (double) pixelPerUnitY) + 1, 1ul);

    glUniform2ui(glGetUniformLocation(glHandler->getProgram(), "resolution"), resolutionX, resolutionY);
    glUniform1ui(glGetUniformLocation(glHandler->getProgram(), "numberOfPoints"), pointCloud->numberOfPoints);

    auto counts = new GLuint[resolutionX * resolutionY];

    for (unsigned long i = 0; i < resolutionX * resolutionY; i++) counts[i] = 0u;
    glHandler->dataToBuffer(GLHandler::EFTDEM_SORTED_POINT_COUNT_BUFFER,
                            (long) (sizeof(GLuint) * resolutionX * resolutionY),
                            counts, GL_STREAM_READ);

    delete[] counts;

    glHandler->dataToBuffer(GLHandler::EFTDEM_RAW_POINT_INDEX_BUFFER, (long long) (pointCloud->numberOfPoints * sizeof(GLuint)), nullptr, GL_STATIC_DRAW);
    glHandler->bindBuffer(GLHandler::EFTDEM_UNBIND);

    auto workgroupSize = (unsigned int) std::ceil(std::sqrt(pointCloud->numberOfPoints));
    glDispatchCompute((GLuint) std::ceil(workgroupSize / 8.), (GLuint) std::ceil(workgroupSize / 8.), 1);

    if (!generateOutput) {
        GLHandler::waitForShaderStorageIntegrity();
        return new pointGrid{.points = {},
                .resolutionX = resolutionX,
                .resolutionY = resolutionY,
                .min = min,
                .max = max,
                .numberOfPoints = pointCloud->numberOfPoints};
    }

    std::vector<unsigned int> gridIndices(pointCloud->numberOfPoints);
    glHandler->dataFromBuffer(GLHandler::EFTDEM_RAW_POINT_INDEX_BUFFER, 0, (long long) (pointCloud->numberOfPoints * sizeof(GLuint)), gridIndices.data());

    auto grid = new pointGrid{.points = std::vector<std::vector<point>>(resolutionX * resolutionY),
                      .resolutionX = resolutionX,
                      .resolutionY = resolutionY,
                      .min = min,
                      .max = max,
                      .numberOfPoints = pointCloud->numberOfPoints};

    for (auto i : gridIndices) {
        if (i > grid->points.size()) std::cout << "Size: " << grid->points.size() << " Index: " << i << std::endl;
    }

    for (size_t i = 0; i < gridIndices.size(); i++) {
        grid->points.at(gridIndices.at(i)).emplace_back(normalizeValue(pointCloud->groundPoints.at(i), min, max));
    }

    return grid;
}