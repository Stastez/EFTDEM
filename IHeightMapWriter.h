#ifndef EFTDEM_IHEIGHTMAPWRITER_H
#define EFTDEM_IHEIGHTMAPWRITER_H

#include "IPipelineComponent.h"
#include <string>

/**
 * The last pipeline component, immediately succeeding an IHeightMapFiller. This takes a heightMap and exports it to disk.
 */
class IHeightMapWriter : public IPipelineComponent {
protected:
    /**
     * The path to where the height map should be written.
     */
    std::string destinationDEM;
public:
    /**
     * Use the currently attached IHeightMapWriter to write the heightMap given by map to disk.
     * @param generateOutput Whether to generateOutput. Must be true
     */
    virtual void apply(const heightMap *map, bool generateOutput) = 0;
    virtual void setDestinationDEM(const std::string& newDestinationDEM) = 0;
    virtual void writeRGB(std::vector<std::vector<int>> data, int resolutionX, int resolutionY) = 0;
};


#endif //EFTDEM_IHEIGHTMAPWRITER_H
