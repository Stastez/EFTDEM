#ifndef EFTDEM_GRADIENTBASEDFILLER_H
#define EFTDEM_GRADIENTBASEDFILLER_H

#include "IKernelBasedFilter.h"

class GradientBasedFiller : public IKernelBasedFilter {
private:
    void dispatchCompute(gl::GLint, bool, heightMap *) const;

public:
    GradientBasedFiller(GLHandler *glHandler, unsigned int kernelRadius, unsigned int batchSize);
    ~GradientBasedFiller() override;
    heightMap * apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_GRADIENTBASEDFILLER_H
