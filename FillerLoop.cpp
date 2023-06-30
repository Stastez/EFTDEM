#include "FillerLoop.h"

#include <utility>

FillerLoop::FillerLoop(std::vector<IHeightMapFiller *> distinctFillers) {
    FillerLoop::fillers = std::move(distinctFillers);
    FillerLoop::stageUsesGPU = false;
    for (auto filler : fillers) FillerLoop::stageUsesGPU |= filler->usesGPU();
}

FillerLoop::~FillerLoop() noexcept = default;

heightMap * FillerLoop::apply(heightMap *map, bool generateOutput) {
    auto result = map;

    for (auto filler : fillers)  {
        result = filler->apply(result, generateOutput);
        delete filler;
    }

    return result;
}