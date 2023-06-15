#ifndef EFTDEM_ICLOUDREADER_H
#define EFTDEM_ICLOUDREADER_H

#include "IPipelineComponent.h"
#include <string>

class ICloudReader : public IPipelineComponent {
protected:
    std::string fileName;
public:
    virtual rawPointCloud apply(bool generateOutput) = 0;
};



#endif //EFTDEM_ICLOUDREADER_H
