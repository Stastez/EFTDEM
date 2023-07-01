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
    RadarComparator::glHandler = pipelines.at(0)->glHandler;
    delete configProvider;
}

std::vector<heightMap *> RadarComparator::compareMaps() {
    std::vector<heightMap *> comparisons;
    comparisons.reserve(pipelines.size());

    std::vector<rawPointCloud *> readerReturns(pipelines.size());
    for (auto i = 0ul; i < pipelines.size(); i++) {
        readerReturns.at(i) = pipelines.at(i)->reader->apply(true);
    }

    std::vector<point> mins, maxs;
    for (auto pointCloud : readerReturns) {
        mins.emplace_back(pointCloud->min);
        maxs.emplace_back(pointCloud->max);
    }

    auto absoluteMin = mergePoints(mins).first,
        absoluteMax = mergePoints(maxs).second;

    for (auto pointCloud : readerReturns) {
        pointCloud->min = absoluteMin;
        pointCloud->max = absoluteMax;
    }

    auto bottomMap = pipelines.at(0)->executeAfterReader(readerReturns.at(0));
    auto topMap = emptyHeightMapfromHeightMap(bottomMap);
    topMap->heights = std::vector<double>(bottomMap->resolutionX * bottomMap->resolutionY, 0);

    auto pixelCount = bottomMap->resolutionX * bottomMap->resolutionY;

    auto shaders = glHandler->getShaderPrograms({"compare.glsl"}, true);
    glHandler->setProgram(shaders.at(0));
    gl::glUniform2ui(gl::glGetUniformLocation(glHandler->getProgram(), "resolution"), bottomMap->resolutionX, bottomMap->resolutionY);

    auto heights = new double[pixelCount];

    for (auto i = 0ul; i < pipelines.size(); i++) {
        glHandler->dataToBuffer(GLHandler::EFTDEM_COMPARISON_BUFFER,
                                (long) (sizeof(double) * pixelCount),
                                nullptr, gl::GLenum::GL_STREAM_READ);
        glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                (long) (sizeof(double) * pixelCount),
                                bottomMap->heights.data(), gl::GLenum::GL_STATIC_DRAW);
        glHandler->dataToBuffer(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER,
                                (long) (sizeof(double) * pixelCount),
                                topMap->heights.data(), gl::GLenum::GL_STATIC_DRAW);
        gl::glDispatchCompute((unsigned int) std::ceil((double) bottomMap->resolutionX / 8.), (unsigned int) std::ceil((double) bottomMap->resolutionY / 8.), 1);
        GLHandler::waitForShaderStorageIntegrity();

        glHandler->dataFromBuffer(GLHandler::EFTDEM_COMPARISON_BUFFER,
                                  0,
                                  (long) (sizeof(double) * pixelCount),
                                  heights);

        auto temp = emptyHeightMapfromHeightMap(bottomMap);
        temp->heights = std::vector<double>(heights, heights + pixelCount);
        comparisons.emplace_back(temp);

        if (i < pipelines.size() - 1) {
            topMap = bottomMap;
            glHandler->uninitializeGL();
            delete glHandler;
            glHandler = pipelines.at(i + 1)->glHandler;
            bottomMap = pipelines.at(i + 1)->executeAfterReader(readerReturns.at(i + 1));
        }
    }

    delete[] heights;

    return comparisons;
}

void RadarComparator::writeComparisons(std::vector<heightMap *> comparisons) {
    auto writer = new GTiffWriter(true, "");

    for (auto i = 1ul; i < comparisons.size(); i++) {
        writer->setDestinationDEM(destinationPaths.at(i) + "_" + std::to_string(i));
        writer->apply(comparisons.at(i), true);
    }
}