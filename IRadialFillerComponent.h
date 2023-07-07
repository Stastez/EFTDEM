#ifndef EFTDEM_IRADIALFILLERCOMPONENT_H
#define EFTDEM_IRADIALFILLERCOMPONENT_H

#include "IHeightMapFiller.h"

class IRadialFillerComponent : public IHeightMapFiller {
    heightMap * apply(heightMap *map, bool generateOutput) override = 0;
};


#endif //EFTDEM_IRADIALFILLERCOMPONENT_H
