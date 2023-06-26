#ifndef EFTDEM_IPIPELINECOMPONENT_H
#define EFTDEM_IPIPELINECOMPONENT_H

#include "DataStructures.h"

class IPipelineComponent {
protected:
    /**
     * Whether or not the described stage uses OpenGL. If adjacent stages both use OpenGL, the former may choose to forgo
     * returning fully formed return types and only include necessary metadata, as the buffers already set on the GPU may be reused.
     */
    bool stageUsesGPU = false;
public:
    virtual ~IPipelineComponent() = default;
    /**
     * Check if the described stage uses OpenGL. If adjacent stages both use OpenGL, the former may choose to forgo
     * returning fully formed return types and only include necessary metadata, as the buffers already set on the GPU may be reused.
     * @return Whether the the described stage uses OpenGL
     */
    virtual bool usesGPU();
};

inline bool IPipelineComponent::usesGPU() {
    return stageUsesGPU;
}

#endif //EFTDEM_IPIPELINECOMPONENT_H
