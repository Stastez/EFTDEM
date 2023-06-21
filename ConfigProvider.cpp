#include "ConfigProvider.h"
#include "Pipeline.h"
#include "CloudReader.h"
#include "SorterGpu.h"
#include "SorterCPU.h"
#include "RasterizerGpu.h"
#include "RasterizerCpu.h"
#include "ClosingFilter.h"
#include "GTiffWriter.h"
#include "InverseDistanceWeightedFilter.h"

#include <iostream>

ConfigProvider::ConfigProvider(std::string configPath) {
    ConfigProvider::configPath = configPath;
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

std::pair<YAML::Node, bool> ConfigProvider::checkValidityAndReturn(YAML::Node node, bool required) {
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

    auto reader = new CloudReader(
            checkValidityAndReturn(
                    config["CloudReaderOptions"]["pointCloudPath"], true).first.as<std::string>());

    auto pixelPerUnit = checkValidityAndReturn(config["CloudSorterOptions"]["pixelPerUnit"], true).first.as<int>();
    auto sorter = (checkValidityAndReturn(config["CloudSorterOptions"]["useGPU"], true).first.as<bool>()) ?
            (ICloudSorter *) new SorterGPU(pipeline->glHandler, pixelPerUnit) : (ICloudSorter *) new SorterCPU(pixelPerUnit);
    
    auto rasterizer = (checkValidityAndReturn(config["CloudRasterizerOptions"]["useGPU"], true).first) ?
            (ICloudRasterizer *) new RasterizerGPU(pipeline->glHandler) : (ICloudRasterizer *) new RasterizerCPU();
    
    IHeightMapFiller *filler;
    auto fillingAlgorithm = checkValidityAndReturn(config["HeightMapFillerOptions"]["filler"], true).first.as<std::string>();
    if (fillingAlgorithm == "closingFilter") {
        auto kernelRadii = checkValidityAndReturn(config["HeightMapFillerOptions"]["kernelBasedFilterOptions"]["kernelSizes"], true).first.as<std::vector<unsigned int>>();
        auto batchSizeTest = checkValidityAndReturn(config["HeightMapFillerOptions"]["kernelBasedFilterOptions"]["batchSize"], false);
        auto batchSize = (batchSizeTest.second) ? batchSizeTest.first.as<unsigned int>() : 0;
        filler = new ClosingFilter(pipeline->glHandler, kernelRadii, batchSize);
    } else if (fillingAlgorithm == "inverseDistanceWeightedFilter") {
        auto kernelRadii = checkValidityAndReturn(config["HeightMapFillerOptions"]["kernelBasedFilterOptions"]["kernelSizes"], true).first.as<std::vector<unsigned int>>();
        auto batchSizeTest = checkValidityAndReturn(config["HeightMapFillerOptions"]["kernelBasedFilterOptions"]["batchSize"], false);
        auto batchSize = (batchSizeTest.second) ? batchSizeTest.first.as<unsigned int>() : 0;
        filler = new InverseDistanceWeightedFilter(pipeline->glHandler, kernelRadii[0], batchSize);
    } else {
        std::cout << "Unrecognized filling algorithm!" << std::endl;
        exit(Pipeline::EXIT_INVALID_CONFIGURATION);
    }

    auto writeLowDepthTest = checkValidityAndReturn(config["CloudWriterOptions"]["writeLowDepth"], false);
    auto writeLowDepth = (writeLowDepthTest.second) ? writeLowDepthTest.first.as<bool>() : false;
    auto writer = new GTiffWriter(writeLowDepth, checkValidityAndReturn(config["CloudWriterOptions"]["destinationPath"], true).first.as<std::string>());

    pipeline->attachElements(reader, sorter, rasterizer, filler, writer);

    return pipeline;
}