#ifndef EFTDEM_SORTERGPU_H
#define EFTDEM_SORTERGPU_H

#include "ICloudSorter.h"
#include "GLHandler.h"

/**
 * An ICloudSorter that creates a pointGrid from a rawPointCloud using the GPU.
 */
class SorterGPU : public ICloudSorter {
private:
    GLHandler *glHandler;

public:
    explicit SorterGPU(GLHandler *glHandler, unsigned long pixelPerUnitX, unsigned long pixelPerUnitY);
    ~SorterGPU() override;
    pointGrid * apply(rawPointCloud *pointCloud, bool generateOutput) override;
};


#endif //EFTDEM_SORTERGPU_H
