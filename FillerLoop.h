#ifndef EFTDEM_FILLERLOOP_H
#define EFTDEM_FILLERLOOP_H

#include "IHeightMapFiller.h"
#include <vector>

/**
 * An IHeightMapFiller that sequentially executes all other IHeightMapFillers provided to it (e.g. ClosingFilters with radii of 5, 10 and 50).
 */
class FillerLoop : public IHeightMapFiller {
private:
    std::vector<IHeightMapFiller *> fillers;

public:
    explicit FillerLoop(std::vector<IHeightMapFiller *> distinctFillers);
    ~FillerLoop() noexcept override;
    heightMap * apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_FILLERLOOP_H
