
#ifndef EFTDEM_RASTERIZER_H
#define EFTDEM_RASTERIZER_H

#include "dataStructures.h"

class rasterizer {
private:
    static std::pair<unsigned long, unsigned long> calculateGridCoordinates(pointGrid *grid, rawPointCloud *pointCloud, double xCoord, double yCoord);

public:
    static pointGrid rasterizeToPointGrid(rawPointCloud *pointCloud, unsigned long resolutionX, unsigned long resolutionY);
    static heightMap rasterizeToHeightMap(pointGrid *pointGrid);
};


#endif //EFTDEM_RASTERIZER_H
