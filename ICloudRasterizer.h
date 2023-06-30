#ifndef EFTDEM_ICLOUDRASTERIZER_H
#define EFTDEM_ICLOUDRASTERIZER_H

#include "IPipelineComponent.h"

class ICloudRasterizer : public IPipelineComponent {
public:
    /**
     * Use the currently attached ICloudRasterizer to convert a pointGrid to a heightMap.
     * @param pointGrid The sorted pointGrid to be read from. If the current ICloudRasterizer uses OpenGL, the points component is optional if a previous stage
     *      already set the appropriate buffers on the GPU
     * @param generateOutput Whether to return a fully formed heightMap (including the actual height data for each pixel). This may be set to false if the following stage
     *      uses OpenGL and can employ the already set buffers on the GPU
     * @return A fully formed heightMap if generateOutput is true, a heightMap only containing all necessary metadata otherwise
     */
    virtual heightMap * apply(pointGrid *pointGrid, bool generateOutput) = 0;
};


#endif //EFTDEM_ICLOUDRASTERIZER_H
