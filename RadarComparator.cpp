#include "RadarComparator.h"
#include "ConfigProvider.h"
#include "GTiffWriter.h"

#include <utility>

RadarComparator::RadarComparator(std::vector<std::string> configPaths) {
    RadarComparator::configPaths = std::move(configPaths);
    auto configProvider = new ConfigProvider();
    pipelines.reserve(RadarComparator::configPaths.size());
    for (const auto& config : RadarComparator::configPaths) {
        pipelines.emplace_back(configProvider->providePipeline(config));
        destinationPaths.emplace_back(configProvider->getComparisonPath());
    }
    RadarComparator::glHandler = configProvider->getGLHandler();
}

std::vector<heightMap *> RadarComparator::compareMaps() {
    std::vector<heightMap *> comparisons;

    auto bottomMap = pipelines[0]->execute();
    auto topMap = emptyHeightMapfromHeightMap(bottomMap);
    topMap->heights = std::vector<double>(bottomMap->resolutionX * bottomMap->resolutionY, 0);

    auto pixelCount = bottomMap->resolutionX * bottomMap->resolutionY;

    auto shaders = glHandler->getShaderPrograms({"compare.glsl"}, true);
    glHandler->setProgram(shaders[0]);
    gl::glUniform2ui(gl::glGetUniformLocation(glHandler->getProgram(), "resolution"), bottomMap->resolutionX, bottomMap->resolutionY);
    glHandler->dataToBuffer(GLHandler::EFTDEM_COMPARISON_BUFFER,
                            (long) (sizeof(double) * pixelCount),
                            nullptr, gl::GLenum::GL_STREAM_READ);

    for (auto i = 0ul; i < pipelines.size(); i++) {
        glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                (long) (sizeof(double) * pixelCount),
                                bottomMap->heights.data(), gl::GLenum::GL_STATIC_DRAW);
        glHandler->dataToBuffer(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER,
                                (long) (sizeof(double) * pixelCount),
                                topMap->heights.data(), gl::GLenum::GL_STATIC_DRAW);
        gl::glDispatchCompute((unsigned int) std::ceil((double) bottomMap->resolutionX / 8.), (unsigned int) std::ceil((double) bottomMap->resolutionY / 8.), 1);
        GLHandler::waitForShaderStorageIntegrity();

        auto heights = new double[pixelCount];
        glHandler->dataFromBuffer(GLHandler::EFTDEM_COMPARISON_BUFFER,
                                  0,
                                  (long) (sizeof(double) * pixelCount),
                                  heights);

        auto temp = emptyHeightMapfromHeightMap(bottomMap);
        temp->heights = std::vector<double>(heights, heights + pixelCount);
        comparisons.emplace_back(temp);

        if (i < pipelines.size() - 1) {
            glHandler->setProgram(shaders[i + 1]);
            gl::glUniform2ui(gl::glGetUniformLocation(glHandler->getProgram(), "resolution"), bottomMap->resolutionX,
                             bottomMap->resolutionY);
            topMap = bottomMap;
            bottomMap = pipelines[i + 1]->execute();
        }
    }

    return comparisons;
}

void RadarComparator::writeComparisons(std::vector<heightMap> comparisons) {
    auto writer = new GTiffWriter(true, "");

    for (auto i = 0ul; i < comparisons.size(); i++) {
        writer->setDestinationDEM(destinationPaths[i] + "_" + std::to_string(i));
        writer->apply(&comparisons[i], true);
    }
}