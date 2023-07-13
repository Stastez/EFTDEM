#include "RadarComparator.h"
#include "ConfigProvider.h"
#include "GTiffWriter.h"

#include <utility>

RadarComparator::RadarComparator(std::vector<std::string> configPaths) {
    RadarComparator::configPaths = std::move(configPaths);
    configProvider = new ConfigProvider();
    pipelines.reserve(RadarComparator::configPaths.size());
    for (const auto & configPath : RadarComparator::configPaths) {
        pipelines.emplace_back(configProvider->providePipeline(configPath));
        destinationPaths.emplace_back(configProvider->getComparisonPath());
        thresholds.emplace_back(configProvider->getThreshold());
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

void addColor(std::vector<std::vector<int>>& colors, std::vector<int> color) {
    colors.at(0).emplace_back(color.at(0));
    colors.at(1).emplace_back(color.at(1));
    colors.at(2).emplace_back(color.at(2));
}

void
RadarComparator::writeThresholdMaps(const std::vector<heightMap *> comparisons, std::vector<std::string> destinationDEM) {
    std::vector<std::vector<int>> colors(3);

    auto writer = new GTiffWriter(false, "");

    for (auto i = 0ul; i < comparisons.size(); i++) {
        for (auto pos = 0ul; pos < comparisons.at(i)->resolutionX * comparisons.at(i)->resolutionY; pos++) {
            auto height = comparisons.at(i)->heights.at(pos);
            auto integerHeight = (int) (height * 255);

            if (height != 0 && denormalizeValue(height, comparisons.at(i)->min.z, comparisons.at(i)->max.z) < thresholds.at(i)) addColor(colors, {255 - integerHeight, 0, 0});
            else addColor(colors, {integerHeight, integerHeight, integerHeight});
        }

        writer->setDestinationDEM(destinationDEM.at(i) + "_color_" + std::to_string(i));
        writer->writeRGB(colors, (int) comparisons.at(i)->resolutionX, (int) comparisons.at(i)->resolutionY);
        colors = std::vector<std::vector<int>>(3);
    }

    delete writer;
}