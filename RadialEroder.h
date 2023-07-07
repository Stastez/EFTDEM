#ifndef EFTDEM_RADIALERODER_H
#define EFTDEM_RADIALERODER_H

#include "GLHandler.h"
#include "IRadialFillerComponent.h"

class RadialEroder : public IRadialFillerComponent {
private:
    bool flipped;
    GLHandler * glHandler;
    unsigned int batchSize;

public:
    explicit RadialEroder(GLHandler *glHandler, bool flipped, unsigned int batchSize);
    heightMap * apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_RADIALERODER_H
