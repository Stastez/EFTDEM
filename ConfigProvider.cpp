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

#include <iostream>
#include <utility>

ConfigProvider::ConfigProvider() = default;

ConfigProvider::ConfigProvider(std::string configPath) {
    ConfigProvider::configPath = std::move(configPath);
}

YAML::Node ConfigProvider::readConfig() {
    YAML::Node config;

    try {
        config = YAML::LoadFile(configPath);
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        exit(Pipeline::EXIT_INVALID_COMMAND_LINE_ARGUMENTS);
    }

    return config;
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



Pipeline *ConfigProvider::providePipeline() {
    auto config = readConfig();

    auto pipeline = new Pipeline(checkValidityAndReturn(config["OpenGLOptions"]["shaderDirectory"], true).first.as<std::string>());

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
            (ICloudSorter *) new SorterGPU(pipeline->glHandler, pixelPerUnitX, pixelPerUnitY) : (ICloudSorter *) new SorterCPU(pixelPerUnitX, pixelPerUnitY);
    
    auto rasterizer = (checkValidityAndReturn(config["CloudRasterizerOptions"]["useGPU"], true).first.as<bool>()) ?
            (ICloudRasterizer *) new RasterizerGPU(pipeline->glHandler) : (ICloudRasterizer *) new RasterizerCPU();
    
    IHeightMapFiller *filler;
    auto fillingAlgorithm = checkValidityAndReturn(config["HeightMapFillerOptions"]["filler"], true).first.as<std::string>();
    if (fillingAlgorithm == "closingFilter") {
        auto kernelRadii = checkValidityAndReturn(config["HeightMapFillerOptions"]["kernelBasedFilterOptions"]["kernelSizes"], true).first.as<std::vector<unsigned int>>();
        auto batchSizeTest = checkValidityAndReturn(config["HeightMapFillerOptions"]["kernelBasedFilterOptions"]["batchSize"], false);
        auto batchSize = (batchSizeTest.second) ? batchSizeTest.first.as<unsigned int>() : 0;

        std::vector<IHeightMapFiller *> filters;
        filters.reserve(kernelRadii.size());
        for (auto radius : kernelRadii) filters.emplace_back(new ClosingFilter(pipeline->glHandler, radius, batchSize));
        filler = new FillerLoop(filters);
    }
    /*if (fillingAlgorithm == "closingFilter") {
        auto kernelRadii = checkValidityAndReturn(config["HeightMapFillerOptions"]["kernelBasedFilterOptions"]["kernelSizes"], true).first.as<std::vector<unsigned int>>();
        auto batchSizeTest = checkValidityAndReturn(config["HeightMapFillerOptions"]["kernelBasedFilterOptions"]["batchSize"], false);
        auto batchSize = (batchSizeTest.second) ? batchSizeTest.first.as<unsigned int>() : 0;
        filler = new ClosingFilter(pipeline->glHandler, kernelRadii, batchSize);
    }*/ else if (fillingAlgorithm == "inverseDistanceWeightedFilter") {
        auto kernelRadii = checkValidityAndReturn(config["HeightMapFillerOptions"]["kernelBasedFilterOptions"]["kernelSizes"], true).first.as<std::vector<unsigned int>>();
        auto batchSizeTest = checkValidityAndReturn(config["HeightMapFillerOptions"]["kernelBasedFilterOptions"]["batchSize"], false);
        auto batchSize = (batchSizeTest.second) ? batchSizeTest.first.as<unsigned int>() : 0;
        filler = new InverseDistanceWeightedFilter(pipeline->glHandler, kernelRadii[0], batchSize);
    } else {
        std::cout << "Unrecognized filling algorithm!" << std::endl;
        exit(Pipeline::EXIT_INVALID_CONFIGURATION);
    }

    auto writeLowDepthTest = checkValidityAndReturn(config["CloudWriterOptions"]["writeLowDepth"], false);
    auto writeLowDepth = writeLowDepthTest.second && writeLowDepthTest.first.as<bool>();
    auto writer = new GTiffWriter(writeLowDepth, checkValidityAndReturn(config["CloudWriterOptions"]["destinationPath"], true).first.as<std::string>());

    pipeline->attachElements(reader, sorter, rasterizer, filler, writer);

    return pipeline;
}