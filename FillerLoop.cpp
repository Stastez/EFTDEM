#include "FillerLoop.h"

#include <utility>
#include <chrono>
#include <iostream>

FillerLoop::FillerLoop(std::vector<IHeightMapFiller *> distinctFillers) {
    FillerLoop::fillers = std::move(distinctFillers);
    FillerLoop::stageUsesGPU = false;
    for (auto filler : fillers) FillerLoop::stageUsesGPU |= filler->usesGPU();
}

FillerLoop::~FillerLoop() noexcept = default;

heightMap * FillerLoop::apply(heightMap *map, bool generateOutput) {
    auto result = map;
    auto start = std::chrono::high_resolution_clock::now();

    for (auto i = 0ul; i < fillers.size(); i++)  {
        auto oldResult = result;
        std::cout << "Current filler number: " << i << std::endl;
        result = fillers.at(i)->apply(oldResult, generateOutput);

        // Pipeline deletes rasterizerReturn which points to the same data as map
        if (i >= 1) delete oldResult;
        delete fillers.at(i);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for complete closing: " << duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    return result;
}