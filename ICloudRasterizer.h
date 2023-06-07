#ifndef EFTDEM_ICLOUDRASTERIZER_H
#define EFTDEM_ICLOUDRASTERIZER_H

#include "IPipelineComponent.h"

class ICloudRasterizer : public IPipelineComponent {
public:
    virtual heightMap apply() = 0;
};


#endif //EFTDEM_ICLOUDRASTERIZER_H
