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

    for (auto i = 0ul; i < fillers.size(); i++)  {
        auto oldResult = result;
        result = fillers.at(i)->apply(oldResult, generateOutput);

        // Pipeline deletes rasterizerReturn which points to the same data as map
        if (i >= 1) delete oldResult;
        delete fillers.at(i);
    }

    return result;
}