#ifndef EFTDEM_CONFIGPROVIDER_H
#define EFTDEM_CONFIGPROVIDER_H

#include "Pipeline.h"
#include <yaml-cpp/yaml.h>

class ConfigProvider {
private:
    std::string configPath;
    YAML::Node readConfig();
    static std::pair<YAML::Node, bool> checkValidityAndReturn(const YAML::Node& node, bool required);

public:
    explicit ConfigProvider(std::string configPath);
    Pipeline *providePipeline();
};


#endif //EFTDEM_CONFIGPROVIDER_H