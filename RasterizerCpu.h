#ifndef EFTDEM_RASTERIZERCPU_H
#define EFTDEM_RASTERIZERCPU_H


#include "ICloudRasterizer.h"
#include "GLHandler.h"

class RasterizerCPU : ICloudRasterizer {
public:
    heightMap apply(pointGrid *pointGrid) override;
};


#endif //EFTDEM_RASTERIZERCPU_H
