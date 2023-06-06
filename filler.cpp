#include <iostream>
#include "filler.h"
#include "dataStructures.h"
#include "glHandler.h"

heightMap filler::applyClosingFilter(heightMap *map, glHandler *glHandler, unsigned int kernelRadius) {
    using namespace gl;

    std::cout << "Applying closing filter using OpenGL..." << std::endl << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    bool debug = false;

    if (!glHandler->isInitialized(debug)) glHandler->initializeGL(debug);
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
    glUniform1ui(glGetUniformLocation(shader, "kernelRadius"), kernelRadius);

    unsigned int batchSize = 64;
    std::chrono::time_point<std::chrono::steady_clock> startInvocation = std::chrono::high_resolution_clock::now(), endInvocation;
    bool first = true;
    for (unsigned long batchX = 0; batchX < std::ceil(map->resolutionX / (double) batchSize); batchX++) {
        for (unsigned long batchY = 0; batchY < std::ceil(map->resolutionY / (double) batchSize); batchY++) {

            glUniform2ui(glGetUniformLocation(shader, "currentInvocation"), batchX * batchSize, batchY * batchSize);
            glDispatchCompute(batchSize, batchSize, 1);
            auto sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            glFlush();

            if ((unsigned int) (batchX * std::ceil(map->resolutionY / (double) batchSize) + batchY) % 100 == 0 && batchX + batchY != 0) {
                if (!first) {
                    std::cout << "\x1b[2K"; // Delete current line
                    for (int i = 0; i < 5; i++) {
                        std::cout
                                << "\x1b[1A" // Move cursor up one
                                << "\x1b[2K"; // Delete the entire line
                    }
                    std::cout << "\r";
                }

                first = false;

                endInvocation = std::chrono::high_resolution_clock::now();

                unsigned int currentInvocation = batchX * std::ceil(map->resolutionY / (double) batchSize) + batchY,
                        totalInvocations = std::ceil(map->resolutionX / (double) batchSize) *
                                           std::ceil(map->resolutionY / (double) batchSize);

                std::cout << "Batch number: " << currentInvocation
                          << " / " << totalInvocations - 1 << std::endl;

                auto elapsedTime = duration_cast<std::chrono::milliseconds>(endInvocation - startInvocation);

                std::cout << "Elapsed time for 100 invocations: "
                          << elapsedTime << std::endl
                          << "Average elapsed time per invocation: "
                          << elapsedTime / 100
                          << std::endl;
                std::cout << "Estimated time to completion: "
                          << duration_cast<std::chrono::seconds>((elapsedTime / 100) * (totalInvocations - currentInvocation))
                          << std::endl << std::endl;
                startInvocation = std::chrono::high_resolution_clock::now();
            }

            glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
            glDeleteSync(sync);
        }
    }

    heightMap filledMap = emptyHeightMapfromHeightMap(map);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbos[1]);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, filledMap.dataSize, filledMap.heights.data());
    glDeleteBuffers(2, ssbos);

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for closing: " << duration_cast<std::chrono::milliseconds>(end - start) << std::endl;

    return filledMap;
}