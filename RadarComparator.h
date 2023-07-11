#ifndef EFTDEM_RADARCOMPARATOR_H
#define EFTDEM_RADARCOMPARATOR_H

#include "Pipeline.h"
#include "ConfigProvider.h"
#include "IComparator.h"

#include <vector>
#include <string>

class RadarComparator : public IComparator {
private:

public:
    explicit RadarComparator(std::vector<std::string> configPaths);
    std::vector<rawPointCloud *> setupPointClouds() override;


    ~RadarComparator() override;
};


#endif //EFTDEM_RADARCOMPARATOR_H
