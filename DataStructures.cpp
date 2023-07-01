#include "DataStructures.h"
#include "Pipeline.h"
#include <utility>
#include <iostream>

/**
 * Calculate the 1D array coordinate associated with (x,y) in grid g
 * @param g The reference grid
 * @param x
 * @param y
 * @return
 */
unsigned long long calculate1DCoordinate(const pointGrid *g, unsigned long x, unsigned long y) {
    return y * g->resolutionX + x;
}

unsigned long long calculate1DCoordinate(const heightMap *h, unsigned long x, unsigned long y) {
    return y * h->resolutionX + x;
}

/**
 * Check whether the given (x,y) is outside of the associated grid
 * @param g The reference grid
 * @param x
 * @param y
 */
void validateCoordinates(pointGrid *g, unsigned long x, unsigned long y) {
    if (x > g->resolutionX - 1 || y > g->resolutionY - 1) {
        std::cout << "Given coordinates outside of grid! Grid dimensions: (" << g->resolutionX << ", " << g->resolutionY << "), given coordinates: (" << x << ", " << y << ")";
        exit(Pipeline::EXIT_INVALID_FUNCTION_PARAMETERS);
    }
}

/**
 * Returns the vector situated at posiiton (x,y) of grid g
 * @param g The pointGrid to be accessed
 * @param x The x coordinate of the requested position (x in [0, g->resolutionX - 1])
 * @param y The y coordinate of the requested position (y in [0, g->resolutionY - 1])
 * @return
 */
std::vector<point> get(pointGrid *g, unsigned long x, unsigned long y){
    validateCoordinates(g, x, y);
    return g->points.at(calculate1DCoordinate(g, x, y));
}

/**
 * Replaces the vector situated at position (x,y) of grid g with value
 * @param g The pointGrid to be updated
 * @param x The x coordinate of the requested position (x in [0, g->resolutionX - 1])
 * @param y The y coordinate of the requested position (y in [0, g->resolutionY - 1])
 * @param value The replacement vector
 */
void set(pointGrid *g, unsigned long x, unsigned long y, std::vector<point> value){
    validateCoordinates(g, x, y);
    g->points.at(calculate1DCoordinate(g, x, y)) = std::move(value);
}

/**
 * Appends the given point to the vector situated at position (x,y) of grid g
 * @param g The pointGrid to be updated
 * @param x The x coordinate of the requested position (x in [0, g->resolutionX - 1])
 * @param y The y coordinate of the requested position (y in [0, g->resolutionY - 1])
 * @param value The point to be appended
 */
void add(pointGrid *g, unsigned long x, unsigned long y, point value){
    validateCoordinates(g, x, y);
    g->points.at(calculate1DCoordinate(g, x, y)).push_back(value);
}

/**
 * Normalizes the given value from the range [min,max] to [0,1]
 * @param value The value to be normalized
 * @param min The current minimal value of the input
 * @param max The current maximal value of the input
 * @return The normalized value
 */
double normalizeValue(double value, double min, double max){
    if (max == min) return 0;
    return (value-min) / (max-min);
}

/**
 * Reverses the normalization by translating the value from the range of [0,1] to [min,max]
 * @param value The value to be denormalized
 * @param min The target minimal value of the input
 * @param max The target maximal value of the input
 * @return The denormalized value
 */
double denormalizeValue(double value, double min, double max){
    return value * (max-min) + min;
}

point normalizeValue(point value, point min, point max) {
    return point{.x = normalizeValue(value.x, min.x, max.x),
                 .y = normalizeValue(value.y, min.y, max.y),
                 .z = normalizeValue(value.z, min.z, max.z),
                 .intensity = value.intensity};
}

point denormalizeValue(point value, point min, point max) {
    return point{.x = denormalizeValue(value.x, min.x, max.x),
            .y = denormalizeValue(value.y, min.y, max.y),
            .z = denormalizeValue(value.z, min.z, max.z),
            .intensity = value.intensity};
}

heightMap * emptyHeightMapfromPointGrid(pointGrid *grid) {
    return new heightMap{.heights = std::vector<double>(grid->resolutionX * grid->resolutionY),
            .resolutionX = grid->resolutionX,
            .resolutionY = grid->resolutionY,
            .dataSize = static_cast<long long>(sizeof(double) * grid->resolutionX * grid->resolutionY),
            .min = grid->min,
            .max = grid->max};
}

heightMap * emptyHeightMapfromHeightMap(heightMap *map){
    return new heightMap{.heights = std::vector<double>(map->resolutionX * map->resolutionY),
            .resolutionX = map->resolutionX,
            .resolutionY = map->resolutionY,
            .dataSize = map->dataSize,
            .min = map->min,
            .max = map->max };
}

std::pair<point, point> mergePoints(point p1, point p2) {
    auto min = point{
            .x = std::min(p1.x, p2.x),
            .y = std::min(p1.y, p2.y),
            .z = std::min(p1.z, p2.z),
            .intensity = std::min(p1.intensity, p2.intensity)
    };
    auto max = point{
            .x = std::max(p1.x, p2.x),
            .y = std::max(p1.y, p2.y),
            .z = std::max(p1.z, p2.z),
            .intensity = std::max(p1.intensity, p2.intensity)
    };

    return {min, max};
}

std::pair<point, point> mergePoints(const std::vector<point>& points) {
    auto min = point{std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),std::numeric_limits<double>::max(), std::numeric_limits<int>::max()};
    auto max = point{-std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),-std::numeric_limits<double>::max(), std::numeric_limits<int>::min()};

    for (auto point : points) {
        min = mergePoints(min, point).first;
        max = mergePoints(max, point).second;
    }

    return {min, max};
}