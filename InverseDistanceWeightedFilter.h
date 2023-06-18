#ifndef EFTDEM_INVERSEDISTANCEWEIGHTEDFILTER_H
#define EFTDEM_INVERSEDISTANCEWEIGHTEDFILTER_H


#include "DataStructures.h"
#include "GLHandler.h"
#include "IKernelBasedFilter.h"

class InverseDistanceWeightedFilter : public IKernelBasedFilter {
private:
    unsigned int kernelRadius;
public:
    InverseDistanceWeightedFilter(GLHandler *glHandler, unsigned int kernelRadius, unsigned int batchSize);
    heightMap apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_INVERSEDISTANCEWEIGHTEDFILTER_H
