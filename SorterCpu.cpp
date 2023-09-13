#include "SorterCpu.h"
#include <iostream>
#include <cmath>

std::pair<unsigned long, unsigned long> SorterCPU::calculateGridCoordinates(pointGrid *grid, rawPointCloud *pointCloud, double xCoord, double yCoord){
    unsigned long x = std::min(grid->resolutionX - 1, (unsigned long) std::floor(normalizeValue(xCoord, pointCloud->min.x, pointCloud->max.x) * (double) grid->resolutionX));
    unsigned long y = std::min(grid->resolutionY - 1, (unsigned long) std::floor(normalizeValue(yCoord, pointCloud->min.y, pointCloud->max.y) * (double) grid->resolutionY));
    return {x, y};
}

SorterCPU::SorterCPU(unsigned long pixelPerUnitX, unsigned long pixelPerUnitY) {
    SorterCPU::stageUsesGPU = false;
    SorterCPU::pixelPerUnitX = pixelPerUnitX;
    SorterCPU::pixelPerUnitY = pixelPerUnitY;
}

SorterCPU::~SorterCPU() noexcept = default;

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
        std::pair<unsigned long, unsigned long> coords = calculateGridCoordinates(grid, pointCloud, it->x, it->y);
        add(grid, coords.first, coords.second, normalizeValue(*it, pointCloud->min, pointCloud->max));
    }

    grid->numberOfPoints = (unsigned int) pointCloud->groundPoints.size();

    return grid;
}