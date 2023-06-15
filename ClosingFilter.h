#ifndef EFTDEM_CLOSINGFILTER_H
#define EFTDEM_CLOSINGFILTER_H


#include "DataStructures.h"
#include "GLHandler.h"
#include "IHeightMapFiller.h"

class ClosingFilter : public IHeightMapFiller {
private:
    GLHandler *glHandler;
    unsigned int kernelRadius, batchSize;
    heightMap applySingleClosingFilter(heightMap *map, bool generateOutput);

public:
    ClosingFilter(GLHandler *glHandler, unsigned int kernelRadius, unsigned int batchSize);
    void cleanUp() override;
    heightMap apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_CLOSINGFILTER_H
