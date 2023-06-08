#ifndef EFTDEM_ICLOUDREADER_H
#define EFTDEM_ICLOUDREADER_H

#include "IPipelineComponent.h"
#include <string>

class ICloudReader : public IPipelineComponent {
public:
    virtual rawPointCloud apply(const std::string &fileName, bool generateOutput) = 0;
};



#endif //EFTDEM_ICLOUDREADER_H
