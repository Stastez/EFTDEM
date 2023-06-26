#ifndef EFTDEM_CLOSINGFILTER_OLD_H
#define EFTDEM_CLOSINGFILTER_OLD_H


#include "DataStructures.h"
#include "GLHandler.h"
#include "IKernelBasedFilter.h"

class ClosingFilter_Old : public IKernelBasedFilter {
private:
    GLHandler *glHandler;
    unsigned int kernelRadius, batchSize;
    void allocBuffer(GLHandler::bufferIndices buffer, long singleDataSize, long dataCount);

public:
    ClosingFilter_Old(GLHandler *glHandler, unsigned int kernelRadius, unsigned int batchSize);
    ~ClosingFilter_Old() noexcept override;
    heightMap apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_CLOSINGFILTER_OLD_H
