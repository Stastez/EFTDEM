#ifndef EFTDEM_IKERNELBASEDFILTER_H
#define EFTDEM_IKERNELBASEDFILTER_H

#include "GLHandler.h"
#include "IHeightMapFiller.h"

struct bufferSpecifications {
    GLHandler::bufferIndices buffer;
    gl::GLsizeiptr size;
};

class IKernelBasedFilter : public IHeightMapFiller {
protected:
    GLHandler *glHandler;
    unsigned int kernelRadius, batchSize;
    std::vector<std::string> shaderPaths;
    std::vector<bufferSpecifications> interimBufferSpecifications;
    void allocBuffer(GLHandler::bufferIndices buffer, long size);

public:
    ~IKernelBasedFilter() noexcept override;
};



#endif //EFTDEM_IKERNELBASEDFILTER_H