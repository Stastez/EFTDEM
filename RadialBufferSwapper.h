#ifndef EFTDEM_RADIALBUFFERSWAPPER_H
#define EFTDEM_RADIALBUFFERSWAPPER_H

#include "IRadialFillerComponent.h"
#include "GLHandler.h"

class RadialBufferSwapper : public IRadialFillerComponent {
private:
    GLHandler * glHandler;

public:
    explicit RadialBufferSwapper(GLHandler * glHandler);
    ~RadialBufferSwapper();
    heightMap * apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_RADIALBUFFERSWAPPER_H
