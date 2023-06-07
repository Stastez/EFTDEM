#include <iostream>
#include "RasterizerGpu.h"

RasterizerGPU::RasterizerGPU(GLHandler *glHandler) {
    RasterizerGPU::glHandler = glHandler;
}

heightMap RasterizerGPU::apply(pointGrid *pointGrid) {
    using namespace gl;

    std::cout << "Rasterizing points to height map using OpenGL..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    if (!glHandler->isInitialized()) glHandler->initializeGL(false);
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
    heightMap map = emptyHeightMapfromPointGrid(pointGrid);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbos[2]);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, (long long) sizeof(double) * pointGrid->resolutionX * pointGrid->resolutionY, map.heights.data());
    glDeleteBuffers(3, ssbos);

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for averaging: " << duration_cast<std::chrono::milliseconds>(end - start) << std::endl;

    return map;
}