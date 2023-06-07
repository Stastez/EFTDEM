#ifndef EFTDEM_IHEIGHTMAPFILLER_H
#define EFTDEM_IHEIGHTMAPFILLER_H

#include "IPipelineComponent.h"

class IHeightMapFiller : public IPipelineComponent {
public:
    virtual heightMap apply(heightMap *map) = 0;
};


#endif //EFTDEM_IHEIGHTMAPFILLER_H
