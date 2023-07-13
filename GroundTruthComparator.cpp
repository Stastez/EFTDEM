#include "GroundTruthComparator.h"
#include "ConfigProvider.h"
#include "GTiffWriter.h"
#include "TiffPipeline.h"
#include "DataStructures.h"

#include <utility>

GroundTruthComparator::GroundTruthComparator(std::vector<std::string> configPaths) {
    GroundTruthComparator::configPaths = std::move(configPaths);
    configProvider = new ConfigProvider();
    pipelines.reserve(GroundTruthComparator::configPaths.size());
    for (const auto& config : GroundTruthComparator::configPaths) {
        if (config.substr(config.length() - 5, config.length() - 1) == ".tiff") {
            pipelines.emplace_back(new TiffPipeline(config));
        }
        else {
            pipelines.emplace_back(configProvider->providePipeline(config));
            destinationPaths.emplace_back(configProvider->getComparisonPath());
        }
    }
    GroundTruthComparator::glHandler = pipelines.at(1)->getGLHandler();
    delete configProvider;
    compareShaderPath = "compareSquaredError.glsl";
}

GroundTruthComparator::~GroundTruthComparator() {
    delete configProvider;
    for (auto pipeline : pipelines) delete pipeline;
    configProvider = nullptr;
}

std::vector<rawPointCloud *> GroundTruthComparator::setupPointClouds() {
    std::vector<rawPointCloud *> readerReturns(pipelines.size());

    heightMap * map = pipelines.at(0)->execute();
    readerReturns.at(0) = emptyPointCloudFromHeightMap(map);

    for (auto i = 1ul; i < pipelines.size(); i++) {
        readerReturns.at(i) = pipelines.at(i)->getCloudReader()->apply(true);
    }

    auto absoluteMin = map->min,
         absoluteMax = map->max;

    for (auto pointCloud : readerReturns) {
        pointCloud->min = absoluteMin;
        pointCloud->max = absoluteMax;
    }

    return readerReturns;
}