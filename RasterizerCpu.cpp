#include <iostream>
#include "RasterizerCpu.h"

void RasterizerCPU::cleanUp() {

}

/**
 * Creates a heightmap from the given point grid by averaging the points of every grid cell.
 * @param pointGrid The point grid containing the point data, sorted into a grid
 * @param useGPU Whether to use OpenGL GPU-acceleration
 * @param glHandler If GPU-acceleration is used, the GLHandler for creating contexts and reading shaders
 * @return A new heightMap struct
 */
heightMap RasterizerCPU::apply(pointGrid *pointGrid) {
    std::cout << "Rasterizing points to height map using CPU..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    heightMap map = emptyHeightMapfromPointGrid(pointGrid);
    long double sum;

    for (unsigned long long i = 0; i < map.resolutionX * map.resolutionY; i++){
        sum = 0;

        for (auto it = pointGrid->points[i].begin(); it != pointGrid->points[i].end(); it++) sum += it->z;
        map.heights[i] = static_cast<double>(sum / std::max(static_cast<long double>(pointGrid->points[i].size()), (long double) 1));
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for averaging: " << duration_cast<std::chrono::milliseconds>(end - start) << std::endl;

    return map;
}