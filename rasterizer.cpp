
#include <iostream>
#include <chrono>
#include "rasterizer.h"
#include "glHandler.h"
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 300

#include <CL/opencl.hpp>
#include <fstream>

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
heightMap rasterizer::rasterizeToHeightMap(pointGrid *pointGrid, int useGPU = 0, glHandler *glHandler = nullptr) {
    if (useGPU == 1) {
        return rasterizeToHeightMapOpenGL(pointGrid, glHandler);
    } else if (useGPU == 2) {
        return rasterizeToHeightMapOpenCL(pointGrid);
    }

    std::cout << "Rasterizing points to height map using CPU..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    heightMap map = {.heights = new double[pointGrid->resolutionX * pointGrid->resolutionY], .resolutionX = pointGrid->resolutionX, .resolutionY = pointGrid->resolutionY,  .min = pointGrid->min, .max = pointGrid->max};
    long double sum;

    for (unsigned long long i = 0; i < map.resolutionX * map.resolutionY; i++){
        sum = 0;

        for (auto it = pointGrid->points[i].begin(); it != pointGrid->points[i].end(); it++) sum += it->z;
        map.heights[i] = static_cast<double>(sum / static_cast<long double>(pointGrid->points[i].size()));
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "OpenCL: " << duration_cast<std::chrono::nanoseconds>(end - start) << std::endl;

    return map;
}

heightMap rasterizer::rasterizeToHeightMapOpenGL(pointGrid *pointGrid, glHandler *glHandler) {
    std::cout << "Rasterizing points to height map using OpenGL..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

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

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "OpenGL: " << duration_cast<std::chrono::nanoseconds>(end - start) << std::endl;

    return map;
}

heightMap rasterizer::rasterizeToHeightMapOpenCL(pointGrid *pointGrid) {
    using namespace cl;

    std::cout << "Rasterizing points to height map using OpenCL..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    Context context(CL_DEVICE_TYPE_GPU);
    CommandQueue queue(context);

    std::string shaderFile = "../../shaders/test.cl";
    std::ifstream shaderFileStream;
    std::stringstream shaderStream;
    shaderFileStream.open(shaderFile);
    if (!shaderFileStream.is_open()) {
        std::cout << "Specified shader could not be opened: " << shaderFile << std::endl;
        exit(3);
    }
    shaderStream << shaderFileStream.rdbuf();
    auto shaderString = shaderStream.str();

    std::vector<double> heights(pointGrid->numberOfPoints);
    std::vector<unsigned int> offsets(pointGrid->resolutionX * pointGrid->resolutionY);
    std::vector<double> results(pointGrid->resolutionX * pointGrid->resolutionY);

    unsigned int dataPosition = 0;
    for (auto i = 0; i < pointGrid->resolutionX * pointGrid->resolutionY; i++) {
        for (auto point : pointGrid->points[i]) {
            heights[dataPosition++] = point.z;
        }
        offsets[i] = dataPosition;
    }

    Program program(context, shaderString, true);

    Buffer heightBuffer(context, heights.begin(), heights.end(), true),
        offsetBuffer(context, offsets.begin(), offsets.end(), true),
        //resultBuffer(context, CL_MEM_WRITE_ONLY, sizeof(double) * pointGrid->resolutionX * pointGrid->resolutionY);
        resultBuffer(context, results.begin(), results.end(), false);

    NDRange global(pointGrid->resolutionX * pointGrid->resolutionY);

    cl_ulong resX = pointGrid->resolutionX;
    cl_ulong resY = pointGrid->resolutionY;

    EnqueueArgs args(queue, global);

    cl::Kernel averageHeight(program, "averageHeight");
    averageHeight.setArg(0, resX);
    averageHeight.setArg(1, resY);
    averageHeight.setArg(2, heightBuffer);
    averageHeight.setArg(3, offsetBuffer);
    averageHeight.setArg(4, resultBuffer);
    queue.enqueueNDRangeKernel(averageHeight,cl::NullRange, global, cl::NullRange);
    queue.finish();

    copy(queue, resultBuffer, results.begin(), results.end());

    heightMap map = {.heights = new double[pointGrid->resolutionX * pointGrid->resolutionY], .resolutionX = pointGrid->resolutionX, .resolutionY = pointGrid->resolutionY,  .min = pointGrid->min, .max = pointGrid->max};
    for (auto i = 0; i < pointGrid->resolutionX * pointGrid->resolutionY; i++) {
        //if (results [i] > 0) std::cout << results[i] << std::endl;
        map.heights[i] = results[i];
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "OpenCL: " << duration_cast<std::chrono::nanoseconds>(end - start) << std::endl;

    return map;
}