#include "FillerLoop.h"

#include <utility>
#include <chrono>
#include <iostream>

/**
 * Constructs a FillerLoop that will apply all IHeightMapFillers given by distinctFillers sequentially. If any filler
 * does not use OpenGL, the FillerLoop copies this behaviour outwardly.
 * @param distinctFillers A vector containing IHeightMapFillers in the order they are supposed to be applied in
 */
FillerLoop::FillerLoop(std::vector<IHeightMapFiller *> distinctFillers) {
    FillerLoop::fillers = std::move(distinctFillers);
    FillerLoop::stageUsesGPU = true;
    for (auto filler : fillers) FillerLoop::stageUsesGPU &= filler->usesGPU();
}

FillerLoop::~FillerLoop() noexcept = default;

/**
 * Fill the given heightMap using the previously provided IHeightMapFillers sequentially.
 * @param map The heightMap to be filled
 * @param generateOutput Whether or not to return a fully formed heightMap (including the actual heightData for each
 * pixel). This may be set to false if the following stage uses OpenGL and can employ the already set buffers on the GPU
 * @return A fully formed heightMap if generateOutput is true, a heightMap only containing all necessary metadata otherwise
 */
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