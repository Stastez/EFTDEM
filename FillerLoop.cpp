#include "FillerLoop.h"

#include <utility>

FillerLoop::FillerLoop(std::vector<IHeightMapFiller *> distinctFillers) {
    FillerLoop::fillers = std::move(distinctFillers);
    FillerLoop::stageUsesGPU = false;
    for (auto filler : fillers) FillerLoop::stageUsesGPU |= filler->usesGPU();
}

FillerLoop::~FillerLoop() {
    for (auto filler : fillers) delete filler;
}

heightMap FillerLoop::apply(heightMap *map, bool generateOutput) {
    heightMap result = *map;

    for (auto filler : fillers) result = filler->apply(&result, generateOutput);

    return result;
}