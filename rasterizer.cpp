
#include <iostream>
#include "rasterizer.h"

std::pair<unsigned long, unsigned long> rasterizer::calculateGridCoordinates(pointGrid *grid, rawPointCloud *pointCloud, double xCoord, double yCoord){
    unsigned long x, y;
    x = std::min(grid->resolutionX - 1, (unsigned long) floor((xCoord - pointCloud->minX) / (pointCloud->maxX - pointCloud->minX) * grid->resolutionX));
    y = std::min(grid->resolutionY - 1, (unsigned long) floor((yCoord - pointCloud->minY) / (pointCloud->maxY - pointCloud->minY) * grid->resolutionY));
    return {x, y};
}

pointGrid rasterizer::rasterizeToPointGrid(rawPointCloud *pointCloud, unsigned long resolutionX, unsigned long resolutionY) {
    std::cout << "Rasterizing points to grid..." << std::endl;

    pointGrid grid = {.points = new std::vector<point>[resolutionX * resolutionY], .resolutionX = resolutionX, .resolutionY = resolutionY};

    for (auto it = pointCloud->groundPoints->begin(); it != pointCloud->groundPoints->end(); it++){
        std::pair<unsigned long, unsigned long> coords = calculateGridCoordinates(&grid, pointCloud, it->x, it->y);
        add(&grid, coords.first, coords.second, *it);
    }

    return grid;
}

heightMap rasterizer::rasterizeToHeightMap(pointGrid *pointGrid) {
    std::cout << "Rasterizing points to height map..." << std::endl;

    heightMap map = {.heights = new double[pointGrid->resolutionX * pointGrid->resolutionY], .resolutionX = pointGrid->resolutionX, .resolutionY = pointGrid->resolutionY};
    long double sum;

    for (unsigned long long i = 0; i < map.resolutionX * map.resolutionY; i++){
        sum = 0;

        for (auto it = pointGrid->points[i].begin(); it != pointGrid->points[i].end(); it++) sum += it->z;
        map.heights[i] = static_cast<double>(sum / static_cast<long double>(pointGrid->points[i].size()));
    }

    return map;
}