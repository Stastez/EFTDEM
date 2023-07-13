#include "RadarComparator.h"
#include "ConfigProvider.h"
#include "GTiffWriter.h"

#include <utility>

RadarComparator::RadarComparator(std::vector<std::string> configPaths) {
    RadarComparator::configPaths = std::move(configPaths);
    configProvider = new ConfigProvider();
    pipelines.reserve(RadarComparator::configPaths.size());
    for (const auto& config : RadarComparator::configPaths) {
        pipelines.emplace_back(configProvider->providePipeline(config));
        destinationPaths.emplace_back(configProvider->getComparisonPath());
    }
    RadarComparator::glHandler = pipelines.at(0)->getGLHandler();
    compareShaderPath = "compare.glsl";
}

RadarComparator::~RadarComparator() {
    delete configProvider;
    for (auto pipeline : pipelines) delete pipeline;
    configProvider = nullptr;
}

std::vector<rawPointCloud *> RadarComparator::setupPointClouds() {
    std::vector<rawPointCloud *> readerReturns(pipelines.size());
    for (auto i = 0ul; i < pipelines.size(); i++) {
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
    return readerReturns;
}