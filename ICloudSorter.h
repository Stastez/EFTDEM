#ifndef EFTDEM_ICLOUDSORTER_H
#define EFTDEM_ICLOUDSORTER_H

#include "IPipelineComponent.h"

class ICloudSorter : public IPipelineComponent {
protected:
    /**
     * Specifies the resolution of the resulting pointGrid by defining how many grid cells
     * shall be allocated for each unit of distance on the xy-plane.
     */
    unsigned long pixelPerUnit;
public:
    /**
     * Use the currently attached ICloudSorter to sort the given point cloud into a pointGrid of the previously specified resolution.
     * @param pointCloud The rawPointCloud to be sorted.
     * @param generateOutput Whether to return a fully formed pointGrid (including the actual points). This may be set to false if the following stage
     *      uses OpenGL and can employ the already set buffers on the GPU
     * @return A fully formed pointGrid if generateOutput is true, a pointGrid only containing all necessary metadata otherwise
     */
    virtual pointGrid apply(rawPointCloud *pointCloud, bool generateOutput) = 0;
};


#endif //EFTDEM_ICLOUDSORTER_H
