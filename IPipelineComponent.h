#ifndef EFTDEM_IPIPELINECOMPONENT_H
#define EFTDEM_IPIPELINECOMPONENT_H

#include "DataStructures.h"

class IPipelineComponent {
private:
    bool defaultComponent = false;

public:
    bool isDefaultComponent() const;
    IPipelineComponent();
    virtual IPipelineComponentReturn apply() const = 0;
};

inline bool IPipelineComponent::isDefaultComponent() const {
    return defaultComponent;
}

inline IPipelineComponent::IPipelineComponent() {
    defaultComponent = true;
}

#endif //EFTDEM_IPIPELINECOMPONENT_H
