#ifndef EFTDEM_RADIALFILLER_H
#define EFTDEM_RADIALFILLER_H

#include "IHeightMapFiller.h"
#include "GLHandler.h"

class RadialFiller : public IHeightMapFiller {
private:
    GLHandler * glHandler;
    bool batched;
    unsigned int batchSize, maxHoleRadius;

public:
    RadialFiller(GLHandler * glHandler, unsigned int maxHoleRadius, bool batched, unsigned int batchSize);
    heightMap * apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_RADIALFILLER_H
