#include <iostream>
#include "IKernelBasedFilter.h"





void IKernelBasedFilter::cleanUp() {

}

heightMap IKernelBasedFilter::applySingleFilter(heightMap *map, bool generateOutput, unsigned int kernelRadius) {
    using namespace gl;

    std::cout << "Applying filter using OpenGL..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    auto shader = glHandler->getShaderPrograms(shaderPaths, true);
    glUseProgram(shader[0]);


    if (!glHandler->getCoherentBufferMask()[GLHandler::EFTDEM_HEIGHTMAP_BUFFER]){
        glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                (long long) sizeof(double) * map->resolutionX * map->resolutionY,
                                map->heights.data(), GL_STATIC_DRAW);
    }
    for (int i=0; i<interimBufferSpecifications.size(); i++){
        glHandler->dataToBuffer(interimBufferSpecifications[i].buffer,
                                (long long) interimBufferSpecifications[i].elementSize * map->resolutionX * map->resolutionY,
                                nullptr, GL_STREAM_READ);
    }

    for (int i=0; i<shader.size(); i++){
        glUseProgram(shader[i]);
        glUniform2ui(glGetUniformLocation(shader[i], "resolution"), map->resolutionX, map->resolutionY);
        glUniform1ui(glGetUniformLocation(shader[i], "kernelRadius"), kernelRadius);
        glHandler->dispatchShader(shader[i], batchSize, map->resolutionX, map->resolutionY);
        glHandler->waitForShaderStorageIntegrity();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for closing: " << duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    if (!generateOutput) return emptyHeightMapfromHeightMap(map);

    heightMap filledMap = emptyHeightMapfromHeightMap(map);
    glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                              0,
                              (long long) sizeof(GLdouble) * filledMap.resolutionX * filledMap.resolutionY,
                              filledMap.heights.data());

    return filledMap;
}