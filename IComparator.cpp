#include "IComparator.h"
#include "ConfigProvider.h"
#include "GTiffWriter.h"
#include "DataStructures.h"

//only for Debug
#include <fstream>
#include <iostream>

#include <utility>


std::vector<heightMap *> IComparator::compareMaps() {
    std::vector<heightMap *> comparisons;
    comparisons.reserve(pipelines.size());

    std::vector<rawPointCloud *> readerReturns = setupPointClouds();

    auto bottomMap = pipelines.at(0)->executeAfterReader(readerReturns.at(0));
    auto topMap = emptyHeightMapFromHeightMap(bottomMap);
    topMap->heights = std::vector<float>(bottomMap->resolutionX * bottomMap->resolutionY, 0);

    auto pixelCount = bottomMap->resolutionX * bottomMap->resolutionY;

    auto shaders = glHandler->getShaderPrograms({compareShaderPath}, true);
    glHandler->setProgram(shaders.at(0));
    gl::glUniform2ui(gl::glGetUniformLocation(glHandler->getProgram(), "resolution"), bottomMap->resolutionX, bottomMap->resolutionY);

    auto heights = new float[pixelCount];

    for (auto i = 0ul; i < pipelines.size(); i++) {
        glHandler->dataToBuffer(GLHandler::EFTDEM_COMPARISON_BUFFER,
                                (long) (sizeof(float) * pixelCount),
                                nullptr, gl::GLenum::GL_STREAM_READ);
        glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                (long) (sizeof(float) * pixelCount),
                                bottomMap->heights.data(), gl::GLenum::GL_STATIC_DRAW);
        glHandler->dataToBuffer(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER,
                                (long) (sizeof(float) * pixelCount),
                                topMap->heights.data(), gl::GLenum::GL_STATIC_DRAW);
        gl::glDispatchCompute((unsigned int) std::ceil((double) bottomMap->resolutionX / 8.), (unsigned int) std::ceil((double) bottomMap->resolutionY / 8.), 1);
        GLHandler::waitForShaderStorageIntegrity();

        glHandler->dataFromBuffer(GLHandler::EFTDEM_COMPARISON_BUFFER,
                                  0,
                                  (long) (sizeof(float) * pixelCount),
                                  heights);

        auto temp = emptyHeightMapFromHeightMap(bottomMap);
        temp->heights = std::vector<float>(heights, heights + pixelCount);
        comparisons.emplace_back(temp);

        if (i < pipelines.size() - 1) {
            delete topMap;
            topMap = bottomMap;
            delete pipelines.at(i);
            pipelines.at(i) = nullptr;
            glHandler = pipelines.at(i + 1)->getGLHandler();
            bottomMap = pipelines.at(i + 1)->executeAfterReader(readerReturns.at(i + 1));
            shaders = glHandler->getShaderPrograms({compareShaderPath}, true);
            glHandler->setProgram(shaders.at(0));
            gl::glUniform2ui(gl::glGetUniformLocation(glHandler->getProgram(), "resolution"), bottomMap->resolutionX, bottomMap->resolutionY);
        }
    }

    delete[] heights;

    return comparisons;
}

void IComparator::writeComparisons(std::vector<heightMap *> comparisons) {
    for (auto i = 0ul; i < comparisons.size(); i++) {
        auto writer = new GTiffWriter(true, "", betterCompression.at(i));

        writer->setDestinationDEM(destinationPaths.at(i) + "_" + std::to_string(i));
        writer->apply(comparisons.at(i), true);
        delete comparisons.at(i);

        delete writer;
    }
}

void IComparator::writeThresholdMaps(const std::vector<heightMap *> &comparisons, const std::vector<std::string> &destinationDEM) {
    (void) comparisons; (void) destinationDEM;
}
