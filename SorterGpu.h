#ifndef EFTDEM_SORTERGPU_H
#define EFTDEM_SORTERGPU_H


#include "ICloudSorter.h"
#include "GLHandler.h"

class SorterGPU : public ICloudSorter {
private:
    GLHandler *glHandler;

public:
    explicit SorterGPU(GLHandler *glHandler, unsigned long pixelPerUnit);
    void cleanUp() override;
    pointGrid apply(rawPointCloud *pointCloud, bool generateOutput) override;
};


#endif //EFTDEM_SORTERGPU_H
