#ifndef EFTDEM_YAMLREADER_H
#define EFTDEM_YAMLREADER_H

#include "IConfigReader.h"

class YAMLReader : public IConfigReader {
public:
    YAMLReader();
    void cleanUp() override;
    YAML::Node apply(std::string configPath) override;
};


#endif //EFTDEM_YAMLREADER_H