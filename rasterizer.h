
#ifndef EFTDEM_RASTERIZER_H
#define EFTDEM_RASTERIZER_H

#include "dataStructures.h"
#include "GLFW/glfw3.h"
#include "glHandler.h"

class rasterizer {
private:
    static std::pair<unsigned long, unsigned long> calculateGridCoordinates(pointGrid *grid, rawPointCloud *pointCloud, double xCoord, double yCoord);
    static heightMap rasterizeToHeightMapGPU(pointGrid *pointGrid, glHandler *glHandler);

public:
    static pointGrid rasterizeToPointGrid(rawPointCloud *pointCloud, unsigned long pixelPerUnit);
    static heightMap rasterizeToHeightMap(pointGrid *pointGrid, bool useGPU, glHandler *glHandler);
};


#endif //EFTDEM_RASTERIZER_H
