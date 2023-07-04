#ifndef EFTDEM_RADARCOMPARATOR_H
#define EFTDEM_RADARCOMPARATOR_H

#include "Pipeline.h"
#include "ConfigProvider.h"

#include <vector>
#include <string>

class RadarComparator {
private:
    std::vector<std::string> configPaths;
    std::vector<std::string> destinationPaths;
    std::vector<Pipeline *> pipelines;
    GLHandler * glHandler;
    ConfigProvider * configProvider;

public:
    explicit RadarComparator(std::vector<std::string> configPaths);
    ~RadarComparator();
    std::vector<heightMap *> compareMaps();
    void writeComparisons(std::vector<heightMap *> comparisons);
};


#endif //EFTDEM_RADARCOMPARATOR_H
