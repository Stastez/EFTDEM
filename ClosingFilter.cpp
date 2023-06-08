#include <iostream>
#include "ClosingFilter.h"
#include "DataStructures.h"
#include "GLHandler.h"

ClosingFilter::ClosingFilter(GLHandler *glHandler, unsigned int kernelRadius, unsigned int batchSize = 0) {
    ClosingFilter::glHandler = glHandler;
    ClosingFilter::kernelRadius = kernelRadius;
    ClosingFilter::batchSize = batchSize;
}

void ClosingFilter::cleanUp() {

}

heightMap ClosingFilter::apply(heightMap *map) {
    using namespace gl;

    std::cout << "Applying closing filter using OpenGL..." << std::endl << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    bool debug = false;

    if (!glHandler->isInitialized(debug)) glHandler->initializeGL(debug);
    std::vector<std::string> shaderPaths;
    shaderPaths.emplace_back("../../shaders/closing.glsl");
    auto shader = glHandler->getShaderPrograms(shaderPaths);
    glUseProgram(shader[0]);

    GLuint ssbos[2];
    glGenBuffers(2, ssbos);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbos[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, map->dataSize, map->heights.data(), GL_STATIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbos[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, map->dataSize, nullptr, GL_STREAM_READ);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glUniform2ui(glGetUniformLocation(shader[0], "resolution"), map->resolutionX, map->resolutionY);
    glUniform1ui(glGetUniformLocation(shader[0], "kernelRadius"), kernelRadius);

    std::chrono::time_point<std::chrono::steady_clock> startInvocation, endInvocation;
    unsigned int localBatchSize = batchSize;
    if (localBatchSize == 0) {
        unsigned int batchSize = 1;

        // find optimal batch size
        while (duration_cast<std::chrono::milliseconds>(endInvocation - startInvocation) <
               std::chrono::milliseconds{500}) {
            startInvocation = std::chrono::high_resolution_clock::now();
            glUniform2ui(glGetUniformLocation(shader[0], "currentInvocation"), batchSize, batchSize);
            glDispatchCompute(batchSize, batchSize, 1);
            auto sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
            glDeleteSync(sync);

            endInvocation = std::chrono::high_resolution_clock::now();

            batchSize *= 2;
        }

        localBatchSize = batchSize;
    }

    bool first = true;
    unsigned int totalInvocations = std::ceil(map->resolutionX * map->resolutionY), currentInvocation;
    for (unsigned long batchX = 0; batchX < map->resolutionX; batchX += localBatchSize * 8) {
        for (unsigned long batchY = 0; batchY < map->resolutionY; batchY += localBatchSize * 4) {
            glUniform2ui(glGetUniformLocation(shader[0], "currentInvocation"), batchX, batchY);
            glDispatchCompute(localBatchSize, localBatchSize, 1);
            auto sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

            currentInvocation = batchX * map->resolutionY + batchY;
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
            auto elapsedTime = endInvocation - startInvocation;

            std::cout << "Batch number: " << currentInvocation
                      << " / " << totalInvocations - 1 << std::endl;
            std::cout << "Elapsed time for " << localBatchSize << " invocations: "
                      << duration_cast<std::chrono::milliseconds>(elapsedTime) << std::endl
                      << "Average elapsed time per invocation: "
                      << duration_cast<std::chrono::milliseconds>(elapsedTime) / localBatchSize
                      << std::endl;
            std::cout << "Batch size: " << localBatchSize << std::endl << std::endl;

            if (duration_cast<std::chrono::milliseconds>(endInvocation - startInvocation) > std::chrono::milliseconds {1000}) {
                localBatchSize /= 2;
                batchX = 0;
                batchY = 0;
            }

            startInvocation = std::chrono::high_resolution_clock::now();

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