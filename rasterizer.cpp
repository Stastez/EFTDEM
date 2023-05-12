#include "rasterizer.h"

std::pair<unsigned long, unsigned long> calculateGridCoordinates(pointGrid *grid, rawPointCloud *pointCloud, double xCoord, double zCoord){
    unsigned long x, z;
    x = std::min(grid->resolutionX - 1, (unsigned long) floor((xCoord - pointCloud->minX) / (pointCloud->maxX - pointCloud->minX) * grid->resolutionX));
    z = std::min(grid->resolutionZ - 1, (unsigned long) floor((zCoord - pointCloud->minZ) / (pointCloud->maxZ - pointCloud->minZ) * grid->resolutionZ));
    return {x,z};
}

pointGrid rasterizer::rasterizeToPointGrid(rawPointCloud pointCloud, unsigned long resolutionX, unsigned long resolutionZ) {
    pointGrid grid = {.points = new std::vector<point>[resolutionX * resolutionZ], .resolutionX = resolutionX, .resolutionZ = resolutionZ};

    for (auto it = pointCloud.groundPoints->begin(); it != pointCloud.groundPoints->end(); it++){
        std::pair<unsigned long, unsigned long> coords = calculateGridCoordinates(&grid, &pointCloud, it->x, it->z);
        add(&grid, coords.first, coords.second, *it);
    }

    return grid;
}

heightMap rasterizer::rasterizeToHeightMap(pointGrid pointGrid) {
    heightMap map = {.heights = new double[pointGrid.resolutionX * pointGrid.resolutionZ], .resolutionX = pointGrid.resolutionX, .resolutionZ = pointGrid.resolutionZ};

    for (unsigned long long i = 0; i < map.resolutionX * map.resolutionZ; i++){
        long double sum = 0;
        for (auto it = pointGrid.points[i].begin(); it != pointGrid.points[i].begin(); it++) sum += it->y;
        map.heights[i] = static_cast<double>(sum / static_cast<long double>(pointGrid.points[i].size()));
    }

    return map;
}