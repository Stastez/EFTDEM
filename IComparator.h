#ifndef EFTDEM_ICOMPERATOR_H
#define EFTDEM_ICOMPERATOR_H

#include "Pipeline.h"
#include "ConfigProvider.h"

#include <vector>
#include <string>

class IComparator {
protected:
    std::vector<std::string> configPaths;
    std::vector<std::string> destinationPaths;
    std::vector<IPipeline *> pipelines;
    GLHandler * glHandler;
    ConfigProvider * configProvider;
    std::string compareShaderPath;
    std::vector<bool> betterCompression;

public:
    virtual ~IComparator() = default;
    std::vector<heightMap *> compareMaps();
    virtual void writeComparisons(std::vector<heightMap *> comparisons);

    virtual std::vector<rawPointCloud *> setupPointClouds() = 0;
};

#endif //EFTDEM_ICOMPERATOR_H
