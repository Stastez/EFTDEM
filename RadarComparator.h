#ifndef EFTDEM_RADARCOMPARATOR_H
#define EFTDEM_RADARCOMPARATOR_H

#include "Pipeline.h"
#include "ConfigProvider.h"
#include "IComparator.h"

#include <vector>
#include <string>

class RadarComparator : public IComparator {
private:
    std::vector<double> thresholds;

public:
    explicit RadarComparator(std::vector<std::string> configPaths);
    std::vector<rawPointCloud *> setupPointClouds() override;
    void writeComparisons(std::vector<heightMap *> comparisons) override;
    void writeThresholdMaps(const std::vector<heightMap *> &comparisons, const std::vector<std::string> &destinationDEM) override;

    ~RadarComparator() override;
};


#endif //EFTDEM_RADARCOMPARATOR_H
