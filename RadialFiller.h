#ifndef EFTDEM_RADIALFILLER_H
#define EFTDEM_RADIALFILLER_H

#include "IHeightMapFiller.h"
#include "GLHandler.h"

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
