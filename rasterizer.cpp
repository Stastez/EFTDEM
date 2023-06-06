
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

    pointGrid grid = {.points = std::vector<std::vector<point>>(resolutionX * resolutionY), .resolutionX = resolutionX, .resolutionY = resolutionY, .min = pointCloud->min, .max = pointCloud->max};

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
        return rasterizeToHeightMapOpenGL(pointGrid, glHandler);
    }

    std::cout << "Rasterizing points to height map using CPU..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    heightMap map = {.heights = std::vector<double>(pointGrid->resolutionX * pointGrid->resolutionY), .resolutionX = pointGrid->resolutionX, .resolutionY = pointGrid->resolutionY,  .min = pointGrid->min, .max = pointGrid->max};
    long double sum;

    for (unsigned long long i = 0; i < map.resolutionX * map.resolutionY; i++){
        sum = 0;

        for (auto it = pointGrid->points[i].begin(); it != pointGrid->points[i].end(); it++) sum += it->z;
        map.heights[i] = static_cast<double>(sum / static_cast<long double>(pointGrid->points[i].size()));
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "CPU: " << duration_cast<std::chrono::milliseconds>(end - start) << std::endl;

    return map;
}

heightMap rasterizer::rasterizeToHeightMapOpenGL(pointGrid *pointGrid, glHandler *glHandler) {
    using namespace gl;

    std::cout << "Rasterizing points to height map using OpenGL..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    glHandler->initializeGL(false);
    auto shader = glHandler->getShader("../../shaders/averageHeight.glsl");
    glUseProgram(shader);

    auto data = new double[pointGrid->numberOfPoints];
    auto chunkBorders = new unsigned int[pointGrid->resolutionX * pointGrid->resolutionY];
    unsigned int dataPosition = 0;
    for (auto i = 0; i < pointGrid->resolutionX * pointGrid->resolutionY; i++) {
        chunkBorders[i] = dataPosition;
        for (auto point : pointGrid->points[i]) {
            data[dataPosition++] = point.z;
        }
    }

    auto ssbos = new GLuint[3];
    glGenBuffers(3, ssbos);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbos[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, (long long) sizeof(double) * pointGrid->numberOfPoints, data, GL_STATIC_DRAW);
    delete[] data;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbos[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, (long long) sizeof(unsigned int) * pointGrid->resolutionX * pointGrid->resolutionY, chunkBorders, GL_STATIC_DRAW);
    delete[] chunkBorders;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbos[2]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, (long long) sizeof(double) * pointGrid->resolutionX * pointGrid->resolutionY, nullptr, GL_STREAM_READ);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glUniform2ui(glGetUniformLocation(shader, "resolution"), pointGrid->resolutionX, pointGrid->resolutionY);

    glDispatchCompute(pointGrid->resolutionX / 8, pointGrid->resolutionY / 8, 1);
    heightMap map = {.heights = std::vector<double>(pointGrid->resolutionX * pointGrid->resolutionY), .resolutionX = pointGrid->resolutionX, .resolutionY = pointGrid->resolutionY,  .min = pointGrid->min, .max = pointGrid->max};
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbos[2]);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, (long long) sizeof(double) * pointGrid->resolutionX * pointGrid->resolutionY, map.heights.data());
    glDeleteBuffers(3, ssbos);

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "OpenGL: " << duration_cast<std::chrono::milliseconds>(end - start) << std::endl;

    return map;
}