#ifndef EFTDEM_DUMMYFILLER_H
#define EFTDEM_DUMMYFILLER_H

#include "IHeightMapFiller.h"

/**
 * An IHeightMapFiller that pipes its input through without filling. Useful for debugging earlier stages.
 */
class DummyFiller : public IHeightMapFiller {
public:
    DummyFiller();
    heightMap * apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_DUMMYFILLER_H
