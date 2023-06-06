#ifndef EFTDEM_FILLER_H
#define EFTDEM_FILLER_H


#include "dataStructures.h"
#include "glHandler.h"

class filler {
public:
    static heightMap applyClosingFilter(heightMap *map, glHandler *glHandler, unsigned int kernelPercentageDivisor);
};


#endif //EFTDEM_FILLER_H
