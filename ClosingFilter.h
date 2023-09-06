#ifndef EFTDEM_CLOSINGFILTER_H
#define EFTDEM_CLOSINGFILTER_H


#include "DataStructures.h"
#include "GLHandler.h"
#include "IKernelBasedFilter.h"

class ClosingFilter : public IKernelBasedFilter {
private:
public:
    ClosingFilter(GLHandler *glHandler, unsigned int kernelRadius, unsigned int batchSize);
    ~ClosingFilter() override;
    heightMap * apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_CLOSINGFILTER_H
