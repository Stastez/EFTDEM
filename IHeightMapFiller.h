#ifndef EFTDEM_IHEIGHTMAPFILLER_H
#define EFTDEM_IHEIGHTMAPFILLER_H

#include "IPipelineComponent.h"

class IHeightMapFiller : public IPipelineComponent {
public:
    /**
     * Use the currently attached IHeightMapFiller to fill holes in the heightMap given by map.
     * @param generateOutput Whether to return a fully formed heightMap (including the actual heightData for each pixel). This may be set to false if the following stage
     *      uses OpenGL and can employ the already set buffers on the GPU
     * @return A fully formed heightMap if generateOutput is true, a heightMap only containing all necessary metadata otherwise
     */
    virtual heightMap * apply(heightMap *map, bool generateOutput) = 0;
};


#endif //EFTDEM_IHEIGHTMAPFILLER_H
