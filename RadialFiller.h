#ifndef EFTDEM_RADIALFILLER_H
#define EFTDEM_RADIALFILLER_H

#include "IHeightMapFiller.h"
#include "GLHandler.h"

/**
 * An IHeightMapFiller that successively fills voids one ring of pixels at a time from the outer bounds of the void.
 */
class RadialFiller : public IHeightMapFiller {
private:
    GLHandler * glHandler;
    unsigned int maxHoleRadius;

public:
    RadialFiller(GLHandler * glHandler, unsigned int maxHoleRadius);
    ~RadialFiller() override;
    heightMap * apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_RADIALFILLER_H
