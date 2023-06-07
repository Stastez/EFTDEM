#ifndef EFTDEM_FILLER_H
#define EFTDEM_FILLER_H


#include "DataStructures.h"
#include "GLHandler.h"

class Filler {
public:
    static heightMap applyClosingFilter(heightMap *map, GLHandler *glHandler, unsigned int kernelRadius);
};


#endif //EFTDEM_FILLER_H
