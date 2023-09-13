#ifndef EFTDEM_RASTERIZERCPU_H
#define EFTDEM_RASTERIZERCPU_H

#include "ICloudRasterizer.h"
#include "GLHandler.h"

/**
 * An ICloudRasterizer that creates a heightMap from a pointGrid using the CPU.
 */
class RasterizerCPU : public ICloudRasterizer {
public:
    RasterizerCPU();
    ~RasterizerCPU() noexcept override;
    heightMap * apply(pointGrid *pointGrid, bool generateOutput) override;
};


#endif //EFTDEM_RASTERIZERCPU_H
