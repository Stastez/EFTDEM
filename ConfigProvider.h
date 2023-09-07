#ifndef EFTDEM_CONFIGPROVIDER_H
#define EFTDEM_CONFIGPROVIDER_H

#include "Pipeline.h"
#include <yaml-cpp/yaml.h>

/**
 * This class can read YAML files and provide Pipeline objects that are configured in the specified way.
 */
class ConfigProvider {
private:
    YAML::Node config;
    std::string configPath;
    YAML::Node readConfig();
    std::pair<YAML::Node, bool> checkValidityAndReturn(const std::vector<std::string> &path, bool required);
    GLHandler *glHandler{};

public:
    ConfigProvider();
    explicit ConfigProvider(std::string configPath);
    ~ConfigProvider();
    Pipeline *providePipeline();
    Pipeline *providePipeline(std::string configPath);

    std::string getDestinationPath();
    double getThreshold();
    bool getBetterCompression();
};


#endif //EFTDEM_CONFIGPROVIDER_H