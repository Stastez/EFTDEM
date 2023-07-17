#include "TiffPipeline.h"
#include "GTiffReader.h"
#include "DataStructures.h"
#include "IHeightMapWriter.h"

#include <utility>

TiffPipeline::TiffPipeline(const std::string& tiffPath, const std::string& shaderDirectory) : TiffPipeline(tiffPath, new GLHandler(shaderDirectory)){}

TiffPipeline::TiffPipeline(const std::string& tiffPath, GLHandler *glHandler){
    TiffPipeline::path = std::move(tiffPath);
    TiffPipeline::glHandler = glHandler;
}

heightMap * TiffPipeline::execute() {
    GTiffReader * reader = new GTiffReader(path);

    map = reader->apply(true);
    delete reader;

    auto pointCloud = new rawPointCloud{
            .groundPoints = std::vector<doublePoint>(),
            .environmentPoints = std::vector<doublePoint>(),
            .min = map->min, .max = map->max,
            .numberOfPoints = 0 };

    return executeAfterReader(pointCloud);
}

heightMap * TiffPipeline::executeAfterReader(rawPointCloud *pointCloud) {

    if (!glHandler->isInitialized(false)) glHandler->initializeGL(false);

    auto normalizedHeights = std::vector<float>(map->resolutionX * map->resolutionY);
    for (auto i = 0ul; i < map->resolutionX * map->resolutionY; i++) {
        normalizedHeights.at(i) = normalizeValue(map->heights.at(i), pointCloud->min.z, pointCloud->max.z);
    }

    return new heightMap{
            .heights = normalizedHeights,
            .resolutionX = map->resolutionX,
            .resolutionY = map->resolutionY,
            .dataSize = (long) (map->resolutionX * map->resolutionY * sizeof(float)),
            .min = pointCloud->min,
            .max = pointCloud->max
    };
}

GLHandler * TiffPipeline::getGLHandler() { return glHandler; }
ICloudReader * TiffPipeline::getCloudReader() { return nullptr; }
ICloudSorter * TiffPipeline::getCloudSorter() { return nullptr; }
IHeightMapWriter * TiffPipeline::getHeightMapWriter() { return nullptr; }
