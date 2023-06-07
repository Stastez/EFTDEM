
#ifndef EFTDEM_RASTERIZER_H
#define EFTDEM_RASTERIZER_H

#include "DataStructures.h"
#include "GLFW/glfw3.h"
#include "GLHandler.h"

class Rasterizer {
private:
    static std::pair<unsigned long, unsigned long> calculateGridCoordinates(pointGrid *grid, rawPointCloud *pointCloud, double xCoord, double yCoord);
    static heightMap rasterizeToHeightMapOpenGL(pointGrid *pointGrid, GLHandler *glHandler);

public:
    static pointGrid rasterizeToPointGrid(rawPointCloud *pointCloud, unsigned long pixelPerUnit);
    static heightMap rasterizeToHeightMap(pointGrid *pointGrid, bool useGPU, GLHandler *glHandler);
};


#endif //EFTDEM_RASTERIZER_H
