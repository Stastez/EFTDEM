#ifndef EFTDEM_RADARCOMPARATOR_H
#define EFTDEM_RADARCOMPARATOR_H

#include "Pipeline.h"
#include "ConfigProvider.h"
#include "IComparator.h"

#include <vector>
#include <string>

/**
 * An IComparator that executes two or more pipelines and exports the difference between adjacent layers as GeoTIFFs.
 * Also writes GeoTIFFs that are colored red wherever the difference subceeds the specified threshold.
 */
class RadarComparator : public IComparator {
private:
    std::vector<double> thresholds;

public:
    explicit RadarComparator(std::vector<std::string> configPaths);
    std::vector<rawPointCloud *> setupPointClouds() override;
    void writeComparisons(std::vector<heightMap *> comparisons) override;
    void writeThresholdMaps(const std::vector<heightMap *> &comparisons);

    ~RadarComparator() override;
};


#endif //EFTDEM_RADARCOMPARATOR_H
