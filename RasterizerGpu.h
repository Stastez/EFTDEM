#ifndef EFTDEM_RASTERIZERGPU_H
#define EFTDEM_RASTERIZERGPU_H

#include "ICloudRasterizer.h"
#include "GLHandler.h"

/**
 * An ICloudRasterizer that creates a heightMap from a pointGrid using the GPU.
 */
class RasterizerGPU : public ICloudRasterizer {
private:
    GLHandler *glHandler;

public:
    explicit RasterizerGPU(GLHandler *glHandler);
    ~RasterizerGPU() override;
    heightMap * apply(pointGrid *pointGrid, bool generateOutput) override;
};


#endif //EFTDEM_RASTERIZERGPU_H
