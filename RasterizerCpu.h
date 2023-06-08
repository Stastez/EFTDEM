#ifndef EFTDEM_RASTERIZERCPU_H
#define EFTDEM_RASTERIZERCPU_H


#include "ICloudRasterizer.h"
#include "GLHandler.h"

class RasterizerCPU : ICloudRasterizer {
public:
    RasterizerCPU();
    void cleanUp() override;
    heightMap apply(pointGrid *pointGrid, bool generateOutput) override;
};


#endif //EFTDEM_RASTERIZERCPU_H
