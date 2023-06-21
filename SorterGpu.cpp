#include "SorterGpu.h"
#include <iostream>
#include <cmath>

SorterGPU::SorterGPU(GLHandler *glHandler, unsigned long pixelPerUnit) {
    SorterGPU::stageUsesGPU = true;
    SorterGPU::glHandler = glHandler;
    SorterGPU::pixelPerUnit = pixelPerUnit;
}

void SorterGPU::cleanUp() {

}

pointGrid SorterGPU::apply(rawPointCloud *pointCloud, bool generateOutput) {
    using namespace gl;

    std::cout << "Sorting points into grid using OpenGL..." << std::endl;

    std::vector<std::string> shaderPaths = {"chunkSortingAndCounting.glsl"};
    auto shaderPrograms = glHandler->getShaderPrograms(shaderPaths, true);
    glHandler->setProgram(shaderPrograms[0]);

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

    unsigned long resolutionX = std::max((unsigned long) std::ceil((pointCloud->max.x - pointCloud->min.x) * (double) pixelPerUnit), 1ul);
    unsigned long resolutionY = std::max((unsigned long) std::ceil((pointCloud->max.y - pointCloud->min.y) * (double) pixelPerUnit), 1ul);

    glUniform2ui(glGetUniformLocation(shaderPrograms[0], "resolution"), resolutionX, resolutionY);
    glUniform1ui(glGetUniformLocation(shaderPrograms[0], "numberOfPoints"), pointCloud->numberOfPoints);

    auto counts = new GLuint[resolutionX * resolutionY];
    for (auto i = 0; i < resolutionX * resolutionY; i++) counts[i] = 0u;
    glHandler->dataToBuffer(GLHandler::EFTDEM_SORTED_POINT_COUNT_BUFFER,
                            (long) (sizeof(GLuint) * resolutionX * resolutionY),
                            counts, GL_STREAM_READ);

    glHandler->dataToBuffer(GLHandler::EFTDEM_RAW_POINT_INDEX_BUFFER, (long long) (pointCloud->numberOfPoints * sizeof(GLuint)), nullptr, GL_STREAM_READ);
    glHandler->bindBuffer(GLHandler::EFTDEM_UNBIND);

    auto workgroupSize = (unsigned int) std::ceil(std::sqrt(pointCloud->numberOfPoints));
    glDispatchCompute(std::ceil(workgroupSize / 8.), std::ceil(workgroupSize / 8.), 1);

    if (!generateOutput) {
        GLHandler::waitForShaderStorageIntegrity();
        return {.points = {},
                .resolutionX = resolutionX,
                .resolutionY = resolutionY,
                .min = min,
                .max = max,
                .numberOfPoints = pointCloud->numberOfPoints};
    }

    std::vector<unsigned int> gridIndices(pointCloud->numberOfPoints);
    glHandler->dataFromBuffer(GLHandler::EFTDEM_RAW_POINT_INDEX_BUFFER, 0, (long long) (pointCloud->numberOfPoints * sizeof(GLuint)), gridIndices.data());

    pointGrid grid = {.points = std::vector<std::vector<point>>(resolutionX * resolutionY),
                      .resolutionX = resolutionX,
                      .resolutionY = resolutionY,
                      .min = min,
                      .max = max,
                      .numberOfPoints = pointCloud->numberOfPoints};

    for (auto i = 0; i < gridIndices.size(); i++) {
        grid.points[gridIndices[i]].emplace_back(normalizeValue(pointCloud->groundPoints[i], min, max));
    }

    return grid;
}