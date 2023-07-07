#ifndef EFTDEM_RADIALBUFFERSWAPPER_H
#define EFTDEM_RADIALBUFFERSWAPPER_H

#include "IRadialFillerComponent.h"
#include "GLHandler.h"

class RadialBufferSwapper : public IRadialFillerComponent {
private:
    GLHandler * glHandler;

public:
    RadialBufferSwapper(GLHandler * glHandler);
    heightMap * apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_RADIALBUFFERSWAPPER_H
