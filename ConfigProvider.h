#ifndef EFTDEM_CONFIGPROVIDER_H
#define EFTDEM_CONFIGPROVIDER_H

#include <yaml-cpp/yaml.h>
#include "Pipeline.h"

class ConfigProvider {
private:
    std::string configPath;
    YAML::Node readConfig();
    std::pair<YAML::Node, bool> checkValidityAndReturn(YAML::Node node, bool required);

public:
    explicit ConfigProvider(std::string configPath);
    Pipeline *providePipeline();
};


#endif //EFTDEM_CONFIGPROVIDER_H