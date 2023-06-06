#include <iostream>
#include "filler.h"
#include "dataStructures.h"
#include "glHandler.h"

heightMap filler::applyClosingFilter(heightMap *map, glHandler *glHandler) {
    using namespace gl;

    std::cout << "Applying closing filter using OpenGL..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();


    if (!glHandler->isInitialized()) glHandler->initializeGL(false);
    auto shader = glHandler->getShader("../../shaders/closing.glsl");
    glUseProgram(shader);

    GLuint ssbos[2];
    glGenBuffers(2, ssbos);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbos[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, map->dataSize, map->heights.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbos[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, map->dataSize, nullptr, GL_STREAM_READ);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glUniform2ui(glGetUniformLocation(shader, "resolution"), map->resolutionX, map->resolutionY);

    glDispatchCompute(map->resolutionX / 8, map->resolutionY / 8, 1);

    heightMap filledMap = emptyHeightMapfromHeightMap(map);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbos[1]);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, filledMap.dataSize, filledMap.heights.data());
    glDeleteBuffers(2, ssbos);

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for closing: " << duration_cast<std::chrono::milliseconds>(end - start) << std::endl;
}