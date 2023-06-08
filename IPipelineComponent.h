#ifndef EFTDEM_IPIPELINECOMPONENT_H
#define EFTDEM_IPIPELINECOMPONENT_H

#include "DataStructures.h"

class IPipelineComponent {
protected:
    bool stageUsesGPU = false;
public:
    virtual void cleanUp() = 0;
    virtual bool usesGPU();
};

inline bool IPipelineComponent::usesGPU() {
    return stageUsesGPU;
}

#endif //EFTDEM_IPIPELINECOMPONENT_H
