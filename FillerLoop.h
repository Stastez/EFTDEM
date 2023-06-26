#ifndef EFTDEM_FILLERLOOP_H
#define EFTDEM_FILLERLOOP_H

#include "IHeightMapFiller.h"
#include <vector>

class FillerLoop : public IHeightMapFiller {
private:
    std::vector<IHeightMapFiller *> fillers;

public:
    explicit FillerLoop(std::vector<IHeightMapFiller *> distinctFillers);
    ~FillerLoop() override;
    heightMap apply(heightMap *map, bool generateOutput);
};


#endif //EFTDEM_FILLERLOOP_H
