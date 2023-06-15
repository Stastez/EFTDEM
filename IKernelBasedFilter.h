#ifndef EFTDEM_IKERNELBASEDFILTER_H
#define EFTDEM_IKERNELBASEDFILTER_H

#include "GLHandler.h"
#include "IHeightMapFiller.h"

struct bufferSpecifications {
    GLHandler::bufferIndices buffer;
    gl::GLsizeiptr elementSize;
};

class IKernelBasedFilter : public IHeightMapFiller {
protected:
    GLHandler *glHandler;
    unsigned int batchSize;
    std::vector<std::string> shaderPaths;
    std::vector<bufferSpecifications> interimBufferSpecifications;

    heightMap applySingleFilter(heightMap *map, bool generateOutput, unsigned int kernelRadius);
public:
    void cleanUp() override;
};



#endif //EFTDEM_IKERNELBASEDFILTER_H