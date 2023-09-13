#include "SorterCpu.h"
#include <iostream>
#include <cmath>

std::pair<unsigned long, unsigned long> SorterCPU::calculateGridCoordinates(pointGrid *grid, double xCoord, double yCoord){
    unsigned long x = std::min(grid->resolutionX - 1, (unsigned long) std::floor(normalizeValue(xCoord, grid->min.x, grid->max.x) * (double) grid->resolutionX));
    unsigned long y = std::min(grid->resolutionY - 1, (unsigned long) std::floor(normalizeValue(yCoord, grid->min.y, grid->max.y) * (double) grid->resolutionY));
    return {x, y};
}

SorterCPU::SorterCPU(unsigned long pixelPerUnitX, unsigned long pixelPerUnitY) {
    SorterCPU::stageUsesGPU = false;
    SorterCPU::pixelPerUnitX = pixelPerUnitX;
    SorterCPU::pixelPerUnitY = pixelPerUnitY;
}

SorterCPU::~SorterCPU() noexcept = default;

/**
 * Creates a sorted grid of points from the points contained in the given rawPointCloud. The grid resolution is calculated
 * using the previously set pixelPerUnit{x,y}.
 * @param pointCloud The rawPointCloud to sort
 * @param generateOutput Whether to create a pointGrid. Should be true
 * @return A pointGrid if generateOutput is true
 */
pointGrid * SorterCPU::apply(rawPointCloud *pointCloud, bool generateOutput) {
    std::cout << "Sorting points into grid using CPU..." << std::endl;

    if (!generateOutput) return {};

    if (resolutionX == 0 || resolutionY == 0){
        resolutionX = std::max((unsigned long) std::ceil((std::abs(pointCloud->max.x - pointCloud->min.x)) * (double) pixelPerUnitX) + 1, 1ul);
        resolutionY = std::max((unsigned long) std::ceil((std::abs(pointCloud->max.y - pointCloud->min.y)) * (double) pixelPerUnitY) + 1, 1ul);
    }

    auto grid = new pointGrid{.points = std::vector<std::vector<floatPoint>>(resolutionX * resolutionY),
                      .resolutionX = resolutionX,
                      .resolutionY = resolutionY,
                      .min = pointCloud->min,
                      .max = pointCloud->max,
                      .numberOfPoints = pointCloud->numberOfPoints};

    for (auto it = pointCloud->groundPoints.begin(); it != pointCloud->groundPoints.end(); it++){
        std::pair<unsigned long, unsigned long> coords = calculateGridCoordinates(grid, it->x, it->y);
        add(grid, coords.first, coords.second, normalizeValue(*it, pointCloud->min, pointCloud->max));
    }

    grid->numberOfPoints = (unsigned int) pointCloud->groundPoints.size();

    return grid;
}