#ifndef EFTDEM_RASTERIZERGPU_H
#define EFTDEM_RASTERIZERGPU_H


#include "ICloudRasterizer.h"
#include "GLHandler.h"

class RasterizerGPU : ICloudRasterizer {
private:
    GLHandler *glHandler;

public:
    explicit RasterizerGPU(GLHandler *glHandler);
    void cleanUp() override;
    heightMap apply(pointGrid *pointGrid) override;
};


#endif //EFTDEM_RASTERIZERGPU_H
