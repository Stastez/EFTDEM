#include <iostream>
#include <cmath>
#include "SorterCPU.h"

std::pair<unsigned long, unsigned long> SorterCPU::calculateGridCoordinates(pointGrid *grid, rawPointCloud *pointCloud, double xCoord, double yCoord){
    unsigned long x, y;
    x = std::min(grid->resolutionX - 1, (unsigned long) std::floor(normalizeValue(xCoord, pointCloud->min.x, pointCloud->max.x) * (double) grid->resolutionX));
    y = std::min(grid->resolutionY - 1, (unsigned long) std::floor(normalizeValue(yCoord, pointCloud->min.y, pointCloud->max.y) * (double) grid->resolutionY));
    return {x, y};
}

SorterCPU::SorterCPU(unsigned long pixelPerUnit) {
    SorterCPU::stageUsesGPU = false;
    SorterCPU::pixelPerUnit = pixelPerUnit;
}

void SorterCPU::cleanUp() {

}

pointGrid SorterCPU::apply(rawPointCloud *pointCloud, bool generateOutput) {
    std::cout << "Sorting points into grid using CPU..." << std::endl;

    if (!generateOutput) return {};

    unsigned long resolutionX = std::max((unsigned long) std::ceil((pointCloud->max.x - pointCloud->min.x) * (double) pixelPerUnit), 1ul);
    unsigned long resolutionY = std::max((unsigned long) std::ceil((pointCloud->max.y - pointCloud->min.y) * (double) pixelPerUnit), 1ul);

    pointGrid grid = {.points = std::vector<std::vector<point>>(resolutionX * resolutionY), .resolutionX = resolutionX, .resolutionY = resolutionY, .min = pointCloud->min, .max = pointCloud->max};

    for (auto it = pointCloud->groundPoints.begin(); it != pointCloud->groundPoints.end(); it++){
        std::pair<unsigned long, unsigned long> coords = calculateGridCoordinates(&grid, pointCloud, it->x, it->y);
        add(&grid, coords.first, coords.second, normalizeValue(*it, pointCloud->min, pointCloud->max));
    }

    grid.numberOfPoints = pointCloud->groundPoints.size();

    return grid;
}