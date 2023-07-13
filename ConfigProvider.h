#ifndef EFTDEM_CONFIGPROVIDER_H
#define EFTDEM_CONFIGPROVIDER_H

#include "Pipeline.h"
#include <yaml-cpp/yaml.h>

class ConfigProvider {
private:
    YAML::Node config;
    std::string configPath;
    YAML::Node readConfig();
    static std::pair<YAML::Node, bool> checkValidityAndReturn(const YAML::Node& node, bool required);
    GLHandler *glHandler{};

public:
    ConfigProvider();
    explicit ConfigProvider(std::string configPath);
    ~ConfigProvider();
    Pipeline *providePipeline();
    Pipeline *providePipeline(std::string configPath);
    GLHandler *getGLHandler();
    std::string getComparisonPath();
    double getThreshold();
};


#endif //EFTDEM_CONFIGPROVIDER_H