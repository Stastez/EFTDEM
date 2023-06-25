#ifndef EFTDEM_CLOSINGFILTER_H
#define EFTDEM_CLOSINGFILTER_H

#include "DataStructures.h"
#include "GLHandler.h"
#include "IKernelBasedFilter.h"

class ClosingFilter : public IKernelBasedFilter {
private:
    std::vector<unsigned int> kernelRadii;
public:
    ClosingFilter(GLHandler *glHandler, std::vector<unsigned int> kernelRadii, unsigned int batchSize);
    heightMap apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_CLOSINGFILTER_H
