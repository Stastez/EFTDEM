#ifndef EFTDEM_GRADIENTBASEDFILLER_H
#define EFTDEM_GRADIENTBASEDFILLER_H

#include "IKernelBasedFilter.h"

/**
 * An IHeightMapFiller that fills voids by interpolating between gradients on the original height map and using them to
 * fill height data.
 */
class GradientBasedFiller : public IKernelBasedFilter {
public:
    GradientBasedFiller(GLHandler *glHandler, unsigned int kernelRadius, unsigned int batchSize);
    ~GradientBasedFiller() override;
    heightMap * apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_GRADIENTBASEDFILLER_H
