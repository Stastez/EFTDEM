#include "IKernelBasedFilter.h"
#include <iostream>

heightMap IKernelBasedFilter::applySingleFilter(heightMap *map, bool generateOutput, unsigned int kernelRadius) {
    using namespace gl;

    std::cout << "Applying filter using OpenGL..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    auto shader = glHandler->getShaderPrograms(shaderPaths, true);
    glUseProgram(shader[0]);


    if (!glHandler->getCoherentBufferMask()[GLHandler::EFTDEM_HEIGHTMAP_BUFFER]){
        glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                (long) (sizeof(double) * map->resolutionX * map->resolutionY),
                                map->heights.data(), GL_STATIC_DRAW);
    }
    for (auto & interimBufferSpecification : interimBufferSpecifications){
        glHandler->dataToBuffer(interimBufferSpecification.buffer,
                                (long) (interimBufferSpecification.size * map->resolutionX * map->resolutionY),
                                nullptr, GL_STREAM_READ);
    }

    for (unsigned int i : shader){
        glUseProgram(i);
        glUniform2ui(glGetUniformLocation(i, "resolution"), map->resolutionX, map->resolutionY);
        glUniform1ui(glGetUniformLocation(i, "kernelRadius"), kernelRadius);
        glHandler->setProgram(i);
        glHandler->dispatchShader(batchSize, map->resolutionX, map->resolutionY);
        GLHandler::waitForShaderStorageIntegrity();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for closing: " << duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    if (!generateOutput) return emptyHeightMapfromHeightMap(map);

    heightMap filledMap = emptyHeightMapfromHeightMap(map);
    glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                              0,
                              (long) (sizeof(GLdouble) * filledMap.resolutionX * filledMap.resolutionY),
                              filledMap.heights.data());

    return filledMap;
}

IKernelBasedFilter::~IKernelBasedFilter() noexcept = default;
