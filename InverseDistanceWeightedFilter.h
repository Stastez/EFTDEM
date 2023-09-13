#ifndef EFTDEM_INVERSEDISTANCEWEIGHTEDFILTER_H
#define EFTDEM_INVERSEDISTANCEWEIGHTEDFILTER_H

#include "DataStructures.h"
#include "GLHandler.h"
#include "IKernelBasedFilter.h"

/**
 * An IHeightMapFiller that closes voids using a kernel that weights decreasingly with distance.
 */
class InverseDistanceWeightedFilter : public IKernelBasedFilter {
public:
    InverseDistanceWeightedFilter(GLHandler *glHandler, unsigned int kernelRadius, unsigned int batchSize);
    ~InverseDistanceWeightedFilter() override;
    heightMap * apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_INVERSEDISTANCEWEIGHTEDFILTER_H
