#include "RadarComparator.h"
#include "ConfigProvider.h"
#include "GTiffWriter.h"
#include "IComparator.h"

#include <utility>

RadarComparator::RadarComparator(std::vector<std::string> configPaths) {
    RadarComparator::configPaths = std::move(configPaths);
    configProvider = new ConfigProvider();
    pipelines.reserve(RadarComparator::configPaths.size());
    for (const auto & configPath : RadarComparator::configPaths) {
        pipelines.emplace_back(configProvider->providePipeline(configPath));
        destinationPaths.emplace_back(configProvider->getDestinationPath());
        thresholds.emplace_back(configProvider->getThreshold());
        betterCompression.emplace_back(configProvider->getBetterCompression());
    }
    RadarComparator::glHandler = pipelines.at(0)->getGLHandler();
    compareShaderPath = "compare.glsl";
}

RadarComparator::~RadarComparator() {
    delete configProvider;
    for (auto pipeline : pipelines) delete pipeline;
    configProvider = nullptr;
}

/**
 * Reads all the point-clouds in the pipelines in pipelines and adds a shared min and max value, so the point-clouds will be scaled and rasterized coherently.
 * @return a Vector containing all the read point-clouds
 */
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

/**
 * Writes the pixel-wise comparisons to a Geotiff-File, with Values falling below a threshold marked in red.
 * @param comparisons a vector of pixel-wise comparisons to be written as Geotiff-Files
 */
void RadarComparator::writeThresholdMaps(const std::vector<heightMap *> &comparisons) {
    std::vector<std::vector<int>> colors(3);

    for (auto i = 0ul; i < comparisons.size(); i++) {
        auto writer = new GTiffWriter(false, "", betterCompression.at(i));

        for (auto pos = 0ul; pos < comparisons.at(i)->resolutionX * comparisons.at(i)->resolutionY; pos++) {
            auto height = comparisons.at(i)->heights.at(pos);
            auto integerHeight = (int) (height * 255);

            if (height != 0 && denormalizeValue(height, comparisons.at(i)->min.z, comparisons.at(i)->max.z) < thresholds.at(i)) addColor(colors, {255 - integerHeight, 0, 0});
            else addColor(colors, {integerHeight, integerHeight, integerHeight});
        }

        writer->setDestinationDEM(destinationPaths.at(i) + "_color_" + std::to_string(i));
        writer->writeRGB(colors, (int) comparisons.at(i)->resolutionX, (int) comparisons.at(i)->resolutionY);
        colors = std::vector<std::vector<int>>(3);

        delete writer;
    }
}

/**
 * Writes the pixel-wise comparisons to a Geotiff-File,
 * as a regular Geotiff-File and an additional Geotiff, with Values falling below a threshold marked in red.
 * @param comparisons a vector of pixel-wise comparisons to be writen as Geotiff-Files
 */
void RadarComparator::writeComparisons(std::vector<heightMap *> comparisons){
    writeThresholdMaps(comparisons);
    IComparator::writeComparisons(comparisons);
}