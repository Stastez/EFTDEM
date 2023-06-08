#ifndef EFTDEM_IPIPELINECOMPONENT_H
#define EFTDEM_IPIPELINECOMPONENT_H

#include "DataStructures.h"

class IPipelineComponent {
public:
    virtual void cleanUp() = 0;
};

#endif //EFTDEM_IPIPELINECOMPONENT_H
