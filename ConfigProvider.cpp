#include "ConfigProvider.h"
#include "Pipeline.h"
#include "MobileMappingReader.h"
#include "SorterGpu.h"
#include "SorterCPU.h"
#include "RasterizerGpu.h"
#include "RasterizerCpu.h"
#include "ClosingFilter.h"
#include "GTiffWriter.h"
#include "InverseDistanceWeightedFilter.h"
#include "GroundRadarReader.h"
#include "FillerLoop.h"
#include "RadialDilator.h"
#include "RadialBufferSwapper.h"
#include "RadialEroder.h"

#include <iostream>
#include <utility>

ConfigProvider::ConfigProvider() = default;

ConfigProvider::ConfigProvider(std::string configPath) {
    ConfigProvider::configPath = std::move(configPath);
}

ConfigProvider::~ConfigProvider() = default;

YAML::Node ConfigProvider::readConfig() {
    YAML::Node localConfig;

    try {
        localConfig = YAML::LoadFile(configPath);
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        exit(Pipeline::EXIT_INVALID_COMMAND_LINE_ARGUMENTS);
    }

    return localConfig;
}

std::pair<YAML::Node, bool> ConfigProvider::checkValidityAndReturn(const YAML::Node& node, bool required) {
    auto returnPair = std::pair{node, true};

    if (!node) {
        returnPair.second = false;
        if (required) {
            std::cout << node.Tag() << " not specified!" << std::endl;
            exit(Pipeline::EXIT_INVALID_CONFIGURATION);
        }
    }

    return returnPair;
}

Pipeline *ConfigProvider::providePipeline(std::string cfgPath) {
    ConfigProvider::configPath = std::move(cfgPath);
    return providePipeline();
}

Pipeline *ConfigProvider::providePipeline() {
    config = readConfig();

    glHandler = new GLHandler(checkValidityAndReturn(config["OpenGLOptions"]["shaderDirectory"], true).first.as<std::string>());

    auto pipeline = new Pipeline(glHandler);

    ICloudReader *reader;
    if (checkValidityAndReturn(config["CloudReaderOptions"]["pointCloudType"], true).first.as<std::string>() == "mobileMapping") {
        reader = new MobileMappingReader(checkValidityAndReturn(config["CloudReaderOptions"]["pointCloudPath"], true).first.as<std::string>());
    } else if (checkValidityAndReturn(config["CloudReaderOptions"]["pointCloudType"], true).first.as<std::string>() == "groundRadar") {
        reader = new GroundRadarReader(checkValidityAndReturn(config["CloudReaderOptions"]["pointCloudPath"], true).first.as<std::string>());
    } else {
        std::cout << "Unrecognized point cloud type!" << std::endl;
        exit(Pipeline::EXIT_INVALID_CONFIGURATION);
    }

    auto pixelPerUnitTest = checkValidityAndReturn(config["CloudSorterOptions"]["pixelPerUnit"], false);
    auto pixelPerUnitXTest = checkValidityAndReturn(config["CloudSorterOptions"]["pixelPerUnitX"], false);
    auto pixelPerUnitYTest = checkValidityAndReturn(config["CloudSorterOptions"]["pixelPerUnitY"], false);

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

    auto sorter = (checkValidityAndReturn(config["CloudSorterOptions"]["useGPU"], true).first.as<bool>()) ?
            (ICloudSorter *) new SorterGPU(glHandler, pixelPerUnitX, pixelPerUnitY) : (ICloudSorter *) new SorterCPU(pixelPerUnitX, pixelPerUnitY);
    
    auto rasterizer = (checkValidityAndReturn(config["CloudRasterizerOptions"]["useGPU"], true).first.as<bool>()) ?
            (ICloudRasterizer *) new RasterizerGPU(glHandler) : (ICloudRasterizer *) new RasterizerCPU();
    
    IHeightMapFiller *filler;
    auto fillingAlgorithm = checkValidityAndReturn(config["HeightMapFillerOptions"]["filler"], true).first.as<std::string>();
    auto kernelRadii = checkValidityAndReturn(config["HeightMapFillerOptions"]["kernelBasedFilterOptions"]["kernelSizes"], true).first.as<std::vector<unsigned int>>();
    auto batchSizeTest = checkValidityAndReturn(config["HeightMapFillerOptions"]["kernelBasedFilterOptions"]["batchSize"], false);
    auto batchSize = (batchSizeTest.second) ? batchSizeTest.first.as<unsigned int>() : 0;

    std::vector<IHeightMapFiller *> filters;
    filters.reserve(kernelRadii.size());
    for (auto radius : kernelRadii) {
        IHeightMapFiller *fillerToAppend;
        if (fillingAlgorithm == "closingFilter")
            fillerToAppend = new ClosingFilter(glHandler, radius, batchSize);
        else if (fillingAlgorithm == "inverseDistanceWeightedFilter")
            fillerToAppend = new InverseDistanceWeightedFilter(glHandler, radius, batchSize);
        else {
            std::cout << fillingAlgorithm << " is either misspelled or not implemented." << std::endl;
            exit(Pipeline::EXIT_INVALID_CONFIGURATION);
        }

        filters.emplace_back(fillerToAppend);
    }

    /*filters = std::vector<IHeightMapFiller *>();
    for (auto i = 0u; i < kernelRadii.at(0); i++) {
        filters.emplace_back(new RadialDilator(glHandler, (bool) (i % 2u), batchSize));
        //filters.emplace_back(new RadialDilator(glHandler, false));
        //filters.emplace_back(new RadialBufferSwapper(glHandler));
    }
    for (auto i = 0u; i < kernelRadii.at(0); i++) {
        filters.emplace_back(new RadialEroder(glHandler, (bool) ((kernelRadii.at(0) + i) % 2u), batchSize));
    }
    if (kernelRadii.at(0) % 2u == 1) filters.emplace_back(new RadialBufferSwapper(glHandler));*/

    filler = new FillerLoop(filters);

    auto writeLowDepthTest = checkValidityAndReturn(config["CloudWriterOptions"]["writeLowDepth"], false);
    auto writeLowDepth = writeLowDepthTest.second && writeLowDepthTest.first.as<bool>();
    auto writer = new GTiffWriter(writeLowDepth, checkValidityAndReturn(config["CloudWriterOptions"]["destinationPath"], true).first.as<std::string>());

    pipeline->attachElements(reader, sorter, rasterizer, filler, writer);

    return pipeline;
}

GLHandler *ConfigProvider::getGLHandler() {
    return glHandler;
}

std::string ConfigProvider::getComparisonPath() {
    return checkValidityAndReturn(config["ComparisonOptions"]["destinationPath"], true).first.as<std::string>();
}