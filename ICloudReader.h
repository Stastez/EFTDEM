#ifndef EFTDEM_ICLOUDREADER_H
#define EFTDEM_ICLOUDREADER_H

#include "IPipelineComponent.h"

class ICloudReader : public IPipelineComponent {
public:
    virtual rawPointCloud apply() = 0;
};



#endif //EFTDEM_ICLOUDREADER_H
