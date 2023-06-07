#ifndef EFTDEM_ICLOUDSORTER_H
#define EFTDEM_ICLOUDSORTER_H

#include "IPipelineComponent.h"

class ICloudSorter : public IPipelineComponent {
public:
    virtual pointGrid apply(rawPointCloud *pointCloud, unsigned long pixelPerUnit) = 0;
};


#endif //EFTDEM_ICLOUDSORTER_H
