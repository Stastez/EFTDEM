#ifndef EFTDEM_IHEIGHTMAPREADER_H
#define EFTDEM_IHEIGHTMAPREADER_H

#include "IPipelineComponent.h"
#include <string>

/**
 * This may be used in a TiffPipeline and reads a GeoTiff from disk into a heightMap.
 */
class IHeightMapReader : public IPipelineComponent {
protected:
    /**
     * The path to where the height map should be read from.
     */
    std::string sourceDEM;
public:
    /**
     * Use the currently attached IHeightMapReader to read the heightMap from the given Path.
     * @param generateOutput Whether to generateOutput. Must be true
     */
    virtual denormalizedHeightMap * apply(bool generateOutput) = 0;
};

#endif //EFTDEM_IHEIGHTMAPREADER_H
