#ifndef EFTDEM_RADIALERODER_H
#define EFTDEM_RADIALERODER_H

#include "GLHandler.h"
#include "IRadialFillerComponent.h"

class RadialEroder : public IRadialFillerComponent {
private:
    bool flipped;
    GLHandler * glHandler;
    unsigned int batchSize;
    bool batched;

public:
    explicit RadialEroder(GLHandler *glHandler, bool flipped, unsigned int batchSize, bool batched);
    heightMap * apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_RADIALERODER_H
