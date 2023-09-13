#include "ConfigProvider.h"
#include "Pipeline.h"
#include "MobileMappingReader.h"
#include "SorterGpu.h"
#include "SorterCpu.h"
#include "RasterizerGpu.h"
#include "RasterizerCpu.h"
#include "ClosingFilter.h"
#include "GTiffWriter.h"
#include "InverseDistanceWeightedFilter.h"
#include "GroundRadarReader.h"
#include "FillerLoop.h"
#include "RadialFiller.h"
#include "DummyFiller.h"
#include "GradientBasedFiller.h"

#include <iostream>
#include <utility>

ConfigProvider::ConfigProvider() = default;

/**
 * Constructs a new ConfigReader while setting the path of the .yaml to read from.
 * @param configPath The path to the .yaml to be read
 */
ConfigProvider::ConfigProvider(std::string configPath) {
    ConfigProvider::configPath = std::move(configPath);
}

ConfigProvider::~ConfigProvider() = default;

/**
 * Tries to read the .yaml pointed to by configPath.
 * @throws std::exception When the given path is not valid, or when there was an error reading the config
 * @return The YAML::Node containing all data found in the given .yaml
 */
YAML::Node ConfigProvider::readConfig() {
    YAML::Node localConfig;

    try {
        localConfig = YAML::LoadFile(configPath);
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        throw std::exception();
    }

    return localConfig;
}

/**
 * Tries to get the child node of ConfigProvider::config by following down the path specified. If the requested child
 * node does not exist and required is true, the program will print out the name of the missing value and exit with
 * EXIT_INVALID_CONFIGURATION.
 * @param path The branch path of the wanted .yaml value as a vector of strings, e.g.:
 * std::vector<std::string>{"OpenGLOptions", "shaderDirectory"}
 * @param required Whether the requested parameter is absolutely required. If true, the program will exit if the
 * requested parameter does not exist in the given .yaml
 * @return A pair containing 1) a YAML::Node containing the requested parameter if it exists, ConfigProvider::config
 * otherwise and 2) a bool that is true, if the requested parameter exists and false otherwise
 */
std::pair<YAML::Node, bool> ConfigProvider::checkValidityAndReturn(const std::vector<std::string> &path, bool required) {
    /*
     * Cloning is necessary here because the copy constructor creates a new Node that points to the same memory, meaning
     * that assignments to current would also assign to config.
    */
    auto current = Clone(config);
    auto returnPair = std::pair{current, true};

    for (const auto& name : path) {
        current = current[name];

        if (!current) {
            returnPair.first = config;
            returnPair.second = false;

            if (required) {
                for (const auto& tempName : path) std::cout << "/" << tempName;
                std::cout << " not specified in config!" << std::endl;
                exit(Pipeline::EXIT_INVALID_CONFIGURATION);
            }

            return returnPair;
        }
    }

    returnPair.first = current;
    return returnPair;
}

/**
 * Sets ConfigProvider::configPath to cfgPath and calls providePipeline()
 * @param cfgPath The path to the .yaml containing the requested configurations
 * @return A pointer to a Pipeline configured in the way specified by the .yaml at cfgPath
 */
Pipeline *ConfigProvider::providePipeline(std::string cfgPath) {
    ConfigProvider::configPath = std::move(cfgPath);
    return providePipeline();
}

/**
 * Builds a Pipeline configured in the way specified by the .yaml pointed to by ConfigProvider::configPath. If the given
 * .yaml contains parameter values that are inconsistent or incorrect, the program may (should) exit with
 * EXIT_INVALID_CONFIGURATION.
 * @return A pointer to a Pipeline configured in the way specified by the .yaml at ConfigProvider::configPath
 */
Pipeline *ConfigProvider::providePipeline() {
    config = readConfig();

    glHandler = new GLHandler(checkValidityAndReturn({"OpenGLOptions", "shaderDirectory"},
                                                     true).first.as<std::string>());

    auto pipeline = new Pipeline(glHandler);

    ICloudReader *reader;
    if (checkValidityAndReturn({"CloudReaderOptions", "pointCloudType"}, true).first.as<std::string>() == "mobileMapping") {
        reader = new MobileMappingReader(checkValidityAndReturn({"CloudReaderOptions", "pointCloudPath"}, true).first.as<std::string>());
    } else if (checkValidityAndReturn({"CloudReaderOptions", "pointCloudType"}, true).first.as<std::string>() == "groundRadar") {
        reader = new GroundRadarReader(checkValidityAndReturn({"CloudReaderOptions", "pointCloudPath"}, true).first.as<std::string>());
    } else {
        std::cout << "Unrecognized point cloud type!" << std::endl;
        exit(Pipeline::EXIT_INVALID_CONFIGURATION);
    }

    auto pixelPerUnitTest = checkValidityAndReturn({"CloudSorterOptions", "pixelPerUnit"}, false);
    auto pixelPerUnitXTest = checkValidityAndReturn({"CloudSorterOptions", "pixelPerUnitX"}, false);
    auto pixelPerUnitYTest = checkValidityAndReturn({"CloudSorterOptions", "pixelPerUnitY"}, false);

    unsigned long pixelPerUnitX, pixelPerUnitY;

    if (pixelPerUnitTest.second) {
        pixelPerUnitX = pixelPerUnitTest.first.as<unsigned long>();
        pixelPerUnitY = pixelPerUnitTest.first.as<unsigned long>();
    } else if (pixelPerUnitXTest.second && pixelPerUnitYTest.second) {
        pixelPerUnitX = pixelPerUnitXTest.first.as<unsigned long>();
        pixelPerUnitY = pixelPerUnitYTest.first.as<unsigned long>();
    } else {
        std::cout << "pixelPerUnit or pixelPerUnitX, pixelPerUnitY not specified!" << std::endl;
        exit(Pipeline::EXIT_INVALID_CONFIGURATION);
    }

    auto sorter = (checkValidityAndReturn({"CloudSorterOptions", "useGPU"}, true).first.as<bool>()) ?
            (ICloudSorter *) new SorterGPU(glHandler, pixelPerUnitX, pixelPerUnitY) : (ICloudSorter *) new SorterCPU(pixelPerUnitX, pixelPerUnitY);
    
    auto rasterizer = (checkValidityAndReturn({"CloudRasterizerOptions", "useGPU"}, true).first.as<bool>()) ?
            (ICloudRasterizer *) new RasterizerGPU(glHandler) : (ICloudRasterizer *) new RasterizerCPU();
    
    IHeightMapFiller *filler;
    auto fillingAlgorithm = checkValidityAndReturn({"HeightMapFillerOptions", "filler"}, true).first.as<std::string>();

    if (fillingAlgorithm == "closingFilter" || fillingAlgorithm == "inverseDistanceWeightedFilter") {
        auto kernelRadii = checkValidityAndReturn(
                {"HeightMapFillerOptions", "kernelBasedFilterOptions", "kernelSizes"},
                true).first.as<std::vector<unsigned int>>();
        auto batchSizeTest = checkValidityAndReturn(
                {"HeightMapFillerOptions", "kernelBasedFilterOptions", "batchSize"}, false);
        auto batchSize = (batchSizeTest.second) ? batchSizeTest.first.as<unsigned int>() : 0;

        std::vector<IHeightMapFiller *> filters;
        filters.reserve(kernelRadii.size());
        for (auto radius: kernelRadii) {
            IHeightMapFiller *fillerToAppend;
            if (fillingAlgorithm == "closingFilter")
                fillerToAppend = new ClosingFilter(glHandler, radius, batchSize);
            else if (fillingAlgorithm == "inverseDistanceWeightedFilter")
                fillerToAppend = new InverseDistanceWeightedFilter(glHandler, radius, batchSize);

            filters.emplace_back(fillerToAppend);
        }

        filler = new FillerLoop(filters);
    } else if (fillingAlgorithm == "radialFiller") {
        auto batchSizeTest = checkValidityAndReturn(
                {"HeightMapFillerOptions", "radialFillerOptions", "batchSize"}, false);
        auto maxHoleRadius = checkValidityAndReturn({"HeightMapFillerOptions", "radialFillerOptions", "maxHoleRadius"},
                                                    true).first.as<unsigned int>();
        filler = new RadialFiller(glHandler, maxHoleRadius);
    } else if (fillingAlgorithm == "gradientFiller") {
        auto kernelRadius = checkValidityAndReturn({"HeightMapFillerOptions", "kernelBasedFilterOptions", "kernelSizes"}, true).first.as<std::vector<unsigned int>>().front();
        auto batchSizeTest = checkValidityAndReturn(
                {"HeightMapFillerOptions", "kernelBasedFilterOptions", "batchSize"}, false);
        auto batchSize = (batchSizeTest.second) ? batchSizeTest.first.as<unsigned int>() : 0;

        filler = new GradientBasedFiller(glHandler, kernelRadius, batchSize);
    } else if (fillingAlgorithm == "dummy") {
        filler = new DummyFiller();
    } else {
        std::cout << fillingAlgorithm << " is either misspelled or not implemented." << std::endl;
        exit(Pipeline::EXIT_INVALID_CONFIGURATION);
    }

    auto writeLowDepthTest = checkValidityAndReturn({"HeightMapWriterOptions", "writeLowDepth"}, false);
    auto writeLowDepth = writeLowDepthTest.second && writeLowDepthTest.first.as<bool>();
    auto betterCompression = checkValidityAndReturn({"HeightMapWriterOptions", "betterCompression"}, false);
    auto writer = new GTiffWriter(writeLowDepth, checkValidityAndReturn({"HeightMapWriterOptions", "destinationPath"},
                                                                        true).first.as<std::string>(),
                                                                        betterCompression.second && betterCompression.first.as<bool>());

    pipeline->attachElements(reader, sorter, rasterizer, filler, writer);

    return pipeline;
}

/**
 * Returns the destinationPath specified in the most recently processed YAML, if it exists.
 * @return The specified destinationPath
 */
std::string ConfigProvider::getDestinationPath() {
    return checkValidityAndReturn({"ComparisonOptions", "destinationPath"}, true).first.as<std::string>();
}

/**
 * Returns the threshold specified in the most recently processed YAML, if it exists.
 * @return The specified threshold
 */
double ConfigProvider::getThreshold() {
    return checkValidityAndReturn({"ComparisonOptions", "threshold"}, true).first.as<double>();
}

/**
 * Returns whether betterCompression is specified and true in the most recently processed YAML. This defaults to false if it is not specified.
 * @return Whether to use better compression
 */
bool ConfigProvider::getBetterCompression() {
    auto betterCompression = checkValidityAndReturn({"HeightMapWriterOptions", "betterCompression"}, false);
    return betterCompression.second && betterCompression.first.as<bool>();
}