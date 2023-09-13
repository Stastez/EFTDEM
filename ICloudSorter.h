#ifndef EFTDEM_ICLOUDSORTER_H
#define EFTDEM_ICLOUDSORTER_H

#include "IPipelineComponent.h"

/**
 * The pipeline stage immediately succeeding ICloudReader. This creates a grid and sorts all points into it using their
 * x,y-coordinates.
 */
class ICloudSorter : public IPipelineComponent {
protected:
    unsigned long resolutionX = 0;
    unsigned long resolutionY = 0;
    unsigned long pixelPerUnitX{};
    unsigned long pixelPerUnitY{};
public:
    /**
     * Use the currently attached ICloudSorter to sort the given point cloud into a pointGrid of the previously specified resolution.
     * @param pointCloud The rawPointCloud to be sorted.
     * @param generateOutput Whether to return a fully formed pointGrid (including the actual points). This may be set to false if the following stage
     *      uses OpenGL and can employ the already set buffers on the GPU
     * @return A fully formed pointGrid if generateOutput is true, a pointGrid only containing all necessary metadata otherwise
     */
    virtual pointGrid * apply(rawPointCloud *pointCloud, bool generateOutput) = 0;
    void setResolution(unsigned long x, unsigned long y) { resolutionX = x; resolutionY = y; };
};


#endif //EFTDEM_ICLOUDSORTER_H
