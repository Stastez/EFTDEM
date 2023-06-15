#include "YAMLReader.h"
#include "Pipeline.h"

#include <iostream>

YAMLReader::YAMLReader() {
    YAMLReader::stageUsesGPU = false;
}

void YAMLReader::cleanUp() {

}

YAML::Node YAMLReader::apply(std::string configPath) {
    YAML::Node config;

    try {
        config = YAML::LoadFile(configPath);
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        exit(Pipeline::EXIT_INVALID_COMMAND_LINE_ARGUMENTS);
    }

    return config;
}