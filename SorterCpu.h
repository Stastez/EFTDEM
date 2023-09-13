#ifndef EFTDEM_SORTERCPU_H
#define EFTDEM_SORTERCPU_H

#include "DataStructures.h"
#include "ICloudSorter.h"

/**
 * An ICloudSorter that creates a pointGrid from a rawPointCloud using the CPU.
 */
class SorterCPU : public ICloudSorter {
private:
    static std::pair<unsigned long, unsigned long> calculateGridCoordinates(pointGrid *grid, double xCoord, double yCoord);

public:
    explicit SorterCPU(unsigned long pixelPerUnitX, unsigned long pixelPerUnitY);
    ~SorterCPU() noexcept override;
    pointGrid * apply(rawPointCloud *pointCloud, bool generateOutput) override;
};


#endif //EFTDEM_SORTERCPU_H
