#ifndef EFTDEM_SORTERCPU_H
#define EFTDEM_SORTERCPU_H


#include "DataStructures.h"
#include "ICloudSorter.h"

class SorterCPU : public ICloudSorter {
private:
    static std::pair<unsigned long, unsigned long> calculateGridCoordinates(pointGrid *grid, rawPointCloud *pointCloud, double xCoord, double yCoord);

public:
    SorterCPU();
    void cleanUp() override;
    pointGrid apply(rawPointCloud *pointCloud, unsigned long pixelPerUnit, bool generateOutput) override;
};


#endif //EFTDEM_SORTERCPU_H
