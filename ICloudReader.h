#ifndef EFTDEM_ICLOUDREADER_H
#define EFTDEM_ICLOUDREADER_H

#include "IPipelineComponent.h"
#include <string>

class ICloudReader : public IPipelineComponent {
protected:
    /**
     * The path of the point cloud to be read from.
     */
    std::string fileName;
public:
    /**
     * Use the currently attached ICloudReader to read the previously specified point cloud from disk.
     * @param generateOutput Whether to return a fully formed rawPointCloud (including the actual points). This may be set to false if the following stage
     *      uses OpenGL and can employ the already set buffers on the GPU
     * @return A fully formed rawPointCloud if generateOutput is true, a rawPointCloud only containing all necessary metadata otherwise
     */
    virtual rawPointCloud apply(bool generateOutput) = 0;
};



#endif //EFTDEM_ICLOUDREADER_H
