#ifndef EFTDEM_RASTERIZERGPU_H
#define EFTDEM_RASTERIZERGPU_H


#include "ICloudRasterizer.h"
#include "GLHandler.h"

class RasterizerGPU : public ICloudRasterizer {
private:
    GLHandler *glHandler;

public:
    explicit RasterizerGPU(GLHandler *glHandler);
    ~RasterizerGPU() override;
    heightMap * apply(pointGrid *pointGrid, bool generateOutput) override;
};


#endif //EFTDEM_RASTERIZERGPU_H
