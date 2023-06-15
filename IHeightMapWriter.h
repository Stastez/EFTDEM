#ifndef EFTDEM_IHEIGHTMAPWRITER_H
#define EFTDEM_IHEIGHTMAPWRITER_H

#include "IPipelineComponent.h"

class IHeightMapWriter : public IPipelineComponent {
protected:
    std::string destinationDEM;
public:
    virtual void apply(const heightMap *map, bool generateOutput) = 0;
};


#endif //EFTDEM_IHEIGHTMAPWRITER_H
