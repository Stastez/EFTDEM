#include "RasterizerCpu.h"
#include <iostream>

RasterizerCPU::RasterizerCPU() {
    RasterizerCPU::stageUsesGPU = false;
}

RasterizerCPU::~RasterizerCPU() noexcept = default;

/**
 * Creates a heightmap from the given point grid by averaging the points of every grid cell.
 * @param pointGrid The point grid containing the point data, sorted into a grid
 * @param generateOutput Whether or not to create output. Should be true
 * @return A new heightMap struct
 */
heightMap * RasterizerCPU::apply(pointGrid *pointGrid, bool generateOutput) {
    std::cout << "Rasterizing points to height map using CPU..." << std::endl;

    if (!generateOutput) return {};

    auto start = std::chrono::high_resolution_clock::now();

    auto map = emptyHeightMapFromPointGrid(pointGrid);
    double sum = 0;

    for (unsigned long long i = 0; i < map->resolutionX * map->resolutionY; i++){
        sum = 0;

        for (auto & it : pointGrid->points.at(i)) sum += it.z;
        map->heights.at(i) = static_cast<float>(sum / std::max(static_cast<double>(pointGrid->points.at(i).size()), (double) 1));
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for averaging: " << duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    return map;
}