#ifndef EFTDEM_DUMMYFILLER_H
#define EFTDEM_DUMMYFILLER_H

#include "IHeightMapFiller.h"

class DummyFiller : public IHeightMapFiller {
public:
    DummyFiller();
    heightMap * apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_DUMMYFILLER_H
