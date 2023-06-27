#ifndef EFTDEM_RADARCOMPARATOR_H
#define EFTDEM_RADARCOMPARATOR_H

#include "Pipeline.h"

#include <vector>
#include <string>

class RadarComparator {
private:
    std::vector<std::string> configPaths;
    std::vector<Pipeline> pipelines;

public:
    explicit RadarComparator(std::vector<std::string> configPaths);
};


#endif //EFTDEM_RADARCOMPARATOR_H
