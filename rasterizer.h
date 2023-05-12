//
// Created by Sandr on 11.05.2023.
//

#ifndef EFTDEM_RASTERIZER_H
#define EFTDEM_RASTERIZER_H


#include "dataStructures.h"

class rasterizer {
public:
    pointGrid rasterizeToPointGrid(rawPointCloud pointCloud, unsigned long resolutionX, unsigned long resolutionZ);
    heightMap rasterizeToHeightMap(pointGrid pointGrid);
};


#endif //EFTDEM_RASTERIZER_H
