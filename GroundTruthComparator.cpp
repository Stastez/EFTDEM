#include "GroundTruthComparator.h"
#include "ConfigProvider.h"
#include "TiffPipeline.h"
#include "DataStructures.h"

#include <utility>
#include "iostream"

GroundTruthComparator::GroundTruthComparator(std::vector<std::string> configPaths) {
    GroundTruthComparator::configPaths = std::move(configPaths);
    configProvider = new ConfigProvider();
    pipelines.reserve(GroundTruthComparator::configPaths.size());

    auto pipeline = configProvider->providePipeline(GroundTruthComparator::configPaths.at(1));
    pipelines.emplace_back(new TiffPipeline(GroundTruthComparator::configPaths.at(0), pipeline->getGLHandler()->getShaderDir()));
    destinationPaths.emplace_back(configProvider->getDestinationPath());
    pipelines.emplace_back(pipeline);
    destinationPaths.emplace_back(configProvider->getDestinationPath());
    for (auto i = 0ul; i < pipelines.size(); i++){
        betterCompression.emplace_back(configProvider->getBetterCompression());
    }

    GroundTruthComparator::glHandler = pipelines.at(0)->getGLHandler();
    compareShaderPath = "compareSquareError.glsl";
}

GroundTruthComparator::~GroundTruthComparator() {
    delete configProvider;
    for (auto pipeline : pipelines) delete pipeline;
    configProvider = nullptr;
}

/**
 * Reads all the point-clouds in the pipelines in pipelines and adds a shared min and max value, so the point-clouds will be scaled and rasterized koherently.
 * @return a Vector containing all the read point-clouds
 */
std::vector<rawPointCloud *> GroundTruthComparator::setupPointClouds() {
    std::vector<rawPointCloud *> readerReturns(pipelines.size());

    heightMap * map = pipelines.at(0)->execute();
    readerReturns.at(0) = emptyPointCloudFromHeightMap(map);

    for (auto i = 1ul; i < pipelines.size(); i++) {
        readerReturns.at(i) = pipelines.at(i)->getCloudReader()->apply(true);
    }

    std::vector<doublePoint> mins, maxs;
    for (auto pointCloud : readerReturns) {
        mins.emplace_back(pointCloud->min);
        maxs.emplace_back(pointCloud->max);
    }

    auto absoluteMin = mergeDoublePoints(mins).first,
            absoluteMax = mergeDoublePoints(maxs).second;

    for (auto pointCloud : readerReturns) {
        pointCloud->min = absoluteMin;
        pointCloud->max = absoluteMax;
    }

    pipelines.at(1)->getCloudSorter()->setResolution(map->resolutionX, map->resolutionY);

    return readerReturns;
}

/**
 * Writes the pixelwise comparisons to a Geotiff-File. And the Mean-Squre-Error to the console.
 * @param comparisons a vector of pixelwise comparisons to be writen as Geotiff-Files.
 */
void GroundTruthComparator::writeComparisons(std::vector<heightMap *> comparisons) {
    std::vector<double> meanSquareErrors(comparisons.size());

    for (auto i = 0ul; i < comparisons.size(); i++) {
        double averageError = 0;
        auto amount = comparisons.at(i)->resolutionX * comparisons.at(i)->resolutionY;
        for (auto j = 0ul; j < amount; j++) {
            averageError += double(comparisons.at(i)->heights.at(j)) / double(amount);
        }
        meanSquareErrors.at(i) = sqrt(averageError);
    }

    for (auto i = 0ul; i < comparisons.size(); i++) {
        std::cout << "Mean-Square-Error: " << meanSquareErrors.at(i) << "\n";
    }

    IComparator::writeComparisons(comparisons);
}