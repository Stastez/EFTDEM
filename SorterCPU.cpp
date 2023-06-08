#include <iostream>
#include "SorterCPU.h"

std::pair<unsigned long, unsigned long> SorterCPU::calculateGridCoordinates(pointGrid *grid, rawPointCloud *pointCloud, double xCoord, double yCoord){
    unsigned long x, y;
    x = std::min(grid->resolutionX - 1, (unsigned long) floor(normalizeValue(xCoord, pointCloud->min.x, pointCloud->max.x) * grid->resolutionX));
    y = std::min(grid->resolutionY - 1, (unsigned long) floor(normalizeValue(yCoord, pointCloud->min.y, pointCloud->max.y) * grid->resolutionY));
    return {x, y};
}

void SorterCPU::cleanUp() {

}

pointGrid SorterCPU::apply(rawPointCloud *pointCloud, unsigned long pixelPerUnit) {
    std::cout << "Sorting points into grid..." << std::endl;

    unsigned long resolutionX = std::max((unsigned long) std::ceil((pointCloud->max.x - pointCloud->min.x) * pixelPerUnit), 1ul);
    unsigned long resolutionY = std::max((unsigned long) std::ceil((pointCloud->max.y - pointCloud->min.y) * pixelPerUnit), 1ul);

    pointGrid grid = {.points = std::vector<std::vector<point>>(resolutionX * resolutionY), .resolutionX = resolutionX, .resolutionY = resolutionY, .min = pointCloud->min, .max = pointCloud->max};

    for (auto it = pointCloud->groundPoints.begin(); it != pointCloud->groundPoints.end(); it++){
        std::pair<unsigned long, unsigned long> coords = calculateGridCoordinates(&grid, pointCloud, it->x, it->y);
        add(&grid, coords.first, coords.second, point(it->x, it->y, normalizeValue(it->z, pointCloud->min.z, pointCloud->max.z), it->intensity));
    }

    grid.numberOfPoints = pointCloud->groundPoints.size();

    return grid;
}