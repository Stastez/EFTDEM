#ifndef EFTDEM_RASTERIZERCPU_H
#define EFTDEM_RASTERIZERCPU_H


#include "ICloudRasterizer.h"
#include "GLHandler.h"

class RasterizerCPU : public ICloudRasterizer {
public:
    RasterizerCPU();
    ~RasterizerCPU() noexcept override;
    heightMap * apply(pointGrid *pointGrid, bool generateOutput) override;
};


#endif //EFTDEM_RASTERIZERCPU_H
