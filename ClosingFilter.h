#ifndef EFTDEM_CLOSINGFILTER_H
#define EFTDEM_CLOSINGFILTER_H


#include "DataStructures.h"
#include "GLHandler.h"
#include "IHeightMapFiller.h"

class ClosingFilter : public IHeightMapFiller {
private:
    GLHandler *glHandler;
    unsigned int kernelRadius, batchSize;

public:
    ClosingFilter(GLHandler *glHandler, unsigned int kernelRadius, unsigned int batchSize);
    heightMap apply(heightMap *map) override;
};


#endif //EFTDEM_CLOSINGFILTER_H
