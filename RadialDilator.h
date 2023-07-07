#ifndef EFTDEM_RADIALDILATOR_H
#define EFTDEM_RADIALDILATOR_H

#include "GLHandler.h"
#include "IRadialFillerComponent.h"

class RadialDilator : public IRadialFillerComponent {
private:
    bool flipped;
    GLHandler * glHandler;
    unsigned int batchSize;

public:
    explicit RadialDilator(GLHandler *glHandler, bool flipped, unsigned int batchSize);
    heightMap * apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_RADIALDILATOR_H
