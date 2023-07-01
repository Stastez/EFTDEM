#include "RasterizerCpu.h"
#include <iostream>

RasterizerCPU::RasterizerCPU() {
    RasterizerCPU::stageUsesGPU = false;
}

RasterizerCPU::~RasterizerCPU() noexcept = default;

/**
 * Creates a heightmap from the given point grid by averaging the points of every grid cell.
 * @param pointGrid The point grid containing the point data, sorted into a grid
 * @param useGPU Whether to use OpenGL GPU-acceleration
 * @param glHandler If GPU-acceleration is used, the GLHandler for creating contexts and reading shaders
 * @return A new heightMap struct
 */
heightMap * RasterizerCPU::apply(pointGrid *pointGrid, bool generateOutput) {
    std::cout << "Rasterizing points to height map using CPU..." << std::endl;

    if (!generateOutput) return {};

    auto start = std::chrono::high_resolution_clock::now();

    auto map = emptyHeightMapfromPointGrid(pointGrid);
    long double sum;

    for (unsigned long long i = 0; i < map->resolutionX * map->resolutionY; i++){
        sum = 0;

        for (auto & it : pointGrid->points.at(i)) sum += it.z;
        map->heights.at(i) = static_cast<double>(sum / std::max(static_cast<long double>(pointGrid->points.at(i).size()), (long double) 1));
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for averaging: " << duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    return map;
}