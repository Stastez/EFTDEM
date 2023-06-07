#ifndef EFTDEM_IHEIGHTMAPWRITER_H
#define EFTDEM_IHEIGHTMAPWRITER_H

#include "IPipelineComponent.h"

class IHeightMapWriter : public IPipelineComponent {
public:
    virtual void apply() = 0;
};


#endif //EFTDEM_IHEIGHTMAPWRITER_H
