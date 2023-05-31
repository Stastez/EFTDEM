
#include <iostream>
#include <chrono>
#include "rasterizer.h"
#include "glHandler.h"

std::pair<unsigned long, unsigned long> rasterizer::calculateGridCoordinates(pointGrid *grid, rawPointCloud *pointCloud, double xCoord, double yCoord){
    unsigned long x, y;
    x = std::min(grid->resolutionX - 1, (unsigned long) floor(normalizeValue(xCoord, pointCloud->min.x, pointCloud->max.x) * grid->resolutionX));
    y = std::min(grid->resolutionY - 1, (unsigned long) floor(normalizeValue(yCoord, pointCloud->min.y, pointCloud->max.y) * grid->resolutionY));
    return {x, y};
}


pointGrid rasterizer::rasterizeToPointGrid(rawPointCloud *pointCloud, unsigned long pixelPerUnit) {
    std::cout << "Sorting points into grid..." << std::endl;

    unsigned long resolutionX = std::max((unsigned long) std::ceil((pointCloud->max.x - pointCloud->min.x) * pixelPerUnit), 1ul);
    unsigned long resolutionY = std::max((unsigned long) std::ceil((pointCloud->max.y - pointCloud->min.y) * pixelPerUnit), 1ul);

    pointGrid grid = {.points = new std::vector<point>[resolutionX * resolutionY], .resolutionX = resolutionX, .resolutionY = resolutionY, .min = pointCloud->min, .max = pointCloud->max};

    for (auto it = pointCloud->groundPoints.begin(); it != pointCloud->groundPoints.end(); it++){
        std::pair<unsigned long, unsigned long> coords = calculateGridCoordinates(&grid, pointCloud, it->x, it->y);
        add(&grid, coords.first, coords.second, point(it->x, it->y, normalizeValue(it->z, pointCloud->min.z, pointCloud->max.z), it->intensity));
    }

    grid.numberOfPoints = pointCloud->groundPoints.size();

    return grid;
}

/**
 * Creates a heightmap from the given point grid by averaging the points of every grid cell.
 * @param pointGrid The point grid containing the point data, sorted into a grid
 * @param useGPU Whether to use OpenGL GPU-acceleration
 * @param glHandler If GPU-acceleration is used, the glHandler for creating contexts and reading shaders
 * @return A new heightMap struct
 */
heightMap rasterizer::rasterizeToHeightMap(pointGrid *pointGrid, bool useGPU = false, glHandler *glHandler = nullptr) {
    if (useGPU) {
        return rasterizeToHeightMapGPU(pointGrid, glHandler);
    }

    std::cout << "Rasterizing points to height map using CPU..." << std::endl;

    heightMap map = {.heights = new double[pointGrid->resolutionX * pointGrid->resolutionY], .resolutionX = pointGrid->resolutionX, .resolutionY = pointGrid->resolutionY,  .min = pointGrid->min, .max = pointGrid->max};
    long double sum;

    for (unsigned long long i = 0; i < map.resolutionX * map.resolutionY; i++){
        sum = 0;

        for (auto it = pointGrid->points[i].begin(); it != pointGrid->points[i].end(); it++) sum += it->z;
        map.heights[i] = static_cast<double>(sum / static_cast<long double>(pointGrid->points[i].size()));
    }

    return map;
}

heightMap rasterizer::rasterizeToHeightMapGPU(pointGrid *pointGrid, glHandler *glHandler) {
    std::cout << "Rasterizing points to height map using GPU..." << std::endl;

    glHandler->initializeGL(false);
    auto shader = glHandler->getShader("../../shaders/test.glsl");
    gl::glUseProgram(shader);

    auto data = new double[pointGrid->numberOfPoints];
    auto chunkBorders = new unsigned int[pointGrid->resolutionX * pointGrid->resolutionY];
    unsigned int dataPosition = 0;
    for (auto i = 0; i < pointGrid->resolutionX * pointGrid->resolutionY; i++) {
        for (auto point : pointGrid->points[i]) {
            data[dataPosition++] = point.z;
        }
        chunkBorders[i] = dataPosition;
    }

    auto ssbos = new gl::GLuint[3];
    gl::glGenBuffers(3, ssbos);
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 0, ssbos[0]);
    gl::glBufferData(gl::GL_SHADER_STORAGE_BUFFER, (long long) sizeof(double) * pointGrid->numberOfPoints, data, gl::GL_STATIC_DRAW);
    delete[] data;
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 1, ssbos[1]);
    gl::glBufferData(gl::GL_SHADER_STORAGE_BUFFER, (long long) sizeof(unsigned int) * pointGrid->resolutionX * pointGrid->resolutionY, chunkBorders, gl::GL_STATIC_DRAW);
    delete[] chunkBorders;
    gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, 2, ssbos[2]);
    gl::glBufferData(gl::GL_SHADER_STORAGE_BUFFER, (long long) sizeof(double) * pointGrid->resolutionX * pointGrid->resolutionY, nullptr, gl::GL_STREAM_READ);
    gl::glBindBuffer(gl::GL_SHADER_STORAGE_BUFFER, 0);

    gl::glUniform2ui(gl::glGetUniformLocation(shader, "resolution"), pointGrid->resolutionX, pointGrid->resolutionY);

    gl::glDispatchCompute(pointGrid->resolutionX / 8, pointGrid->resolutionY / 8, 1);
    heightMap map = {.heights = new double[pointGrid->resolutionX * pointGrid->resolutionY], .resolutionX = pointGrid->resolutionX, .resolutionY = pointGrid->resolutionY,  .min = pointGrid->min, .max = pointGrid->max};
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);

    gl::glBindBuffer(gl::GL_SHADER_STORAGE_BUFFER, ssbos[2]);
    gl::glGetBufferSubData(gl::GL_SHADER_STORAGE_BUFFER, 0, (long long) sizeof(double) * pointGrid->resolutionX * pointGrid->resolutionY, map.heights);
    gl::glDeleteBuffers(3, ssbos);

    return map;
}