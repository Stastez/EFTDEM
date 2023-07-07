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
std::vector<floatPoint> get(pointGrid *g, unsigned long x, unsigned long y){
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
void set(pointGrid *g, unsigned long x, unsigned long y, std::vector<floatPoint> value){
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
void add(pointGrid *g, unsigned long x, unsigned long y, floatPoint value){
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
float normalizeValue(double value, double min, double max){
    if (max == min) return 0;
    return (float) ((value - min) / (max - min));
}

/**
 * Reverses the normalization by translating the value from the range of [0,1] to [min,max]
 * @param value The value to be denormalized
 * @param min The target minimal value of the input
 * @param max The target maximal value of the input
 * @return The denormalized value
 */
double denormalizeValue(float value, double min, double max){
    return (double) (value * (max-min) + min);
}

floatPoint normalizeValue(doublePoint value, doublePoint min, doublePoint max) {
    return floatPoint{.x = normalizeValue(value.x, min.x, max.x),
                 .y = normalizeValue(value.y, min.y, max.y),
                 .z = normalizeValue(value.z, min.z, max.z),
                 .intensity = value.intensity};
}

doublePoint denormalizeValue(floatPoint value, doublePoint min, doublePoint max) {
    return doublePoint{.x = denormalizeValue(value.x, min.x, max.x),
            .y = denormalizeValue(value.y, min.y, max.y),
            .z = denormalizeValue(value.z, min.z, max.z),
            .intensity = value.intensity};
}

heightMap * emptyHeightMapFromPointGrid(pointGrid *grid) {
    return new heightMap{.heights = std::vector<float>(grid->resolutionX * grid->resolutionY),
            .resolutionX = grid->resolutionX,
            .resolutionY = grid->resolutionY,
            .dataSize = static_cast<long>(sizeof(float) * grid->resolutionX * grid->resolutionY),
            .min = grid->min,
            .max = grid->max};
}

heightMap * emptyHeightMapFromHeightMap(heightMap *map){
    return new heightMap{.heights = std::vector<float>(map->resolutionX * map->resolutionY),
            .resolutionX = map->resolutionX,
            .resolutionY = map->resolutionY,
            .dataSize = map->dataSize,
            .min = map->min,
            .max = map->max };
}

std::pair<doublePoint, doublePoint> mergeDoublePoints(doublePoint p1, doublePoint p2) {
    auto min = doublePoint{
            .x = std::min(p1.x, p2.x),
            .y = std::min(p1.y, p2.y),
            .z = std::min(p1.z, p2.z),
            .intensity = std::min(p1.intensity, p2.intensity)
    };
    auto max = doublePoint{
            .x = std::max(p1.x, p2.x),
            .y = std::max(p1.y, p2.y),
            .z = std::max(p1.z, p2.z),
            .intensity = std::max(p1.intensity, p2.intensity)
    };

    return {min, max};
}

std::pair<doublePoint, doublePoint> mergeDoublePoints(const std::vector<doublePoint>& points) {
    auto min = doublePoint{std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<int>::max()};
    auto max = doublePoint{-std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(), std::numeric_limits<int>::min()};

    for (auto point : points) {
        min = mergeDoublePoints(min, point).first;
        max = mergeDoublePoints(max, point).second;
    }

    return {min, max};
}

std::pair<floatPoint, floatPoint> mergeFloatPoints(floatPoint p1, floatPoint p2) {
    auto min = floatPoint{
            .x = std::min(p1.x, p2.x),
            .y = std::min(p1.y, p2.y),
            .z = std::min(p1.z, p2.z),
            .intensity = std::min(p1.intensity, p2.intensity)
    };
    auto max = floatPoint{
            .x = std::max(p1.x, p2.x),
            .y = std::max(p1.y, p2.y),
            .z = std::max(p1.z, p2.z),
            .intensity = std::max(p1.intensity, p2.intensity)
    };

    return {min, max};
}

std::pair<floatPoint, floatPoint> mergeFloatPoints(const std::vector<floatPoint>& points) {
    auto min = floatPoint{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<int>::max()};
    auto max = floatPoint{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), std::numeric_limits<int>::min()};

    for (auto point : points) {
        min = mergeFloatPoints(min, point).first;
        max = mergeFloatPoints(max, point).second;
    }

    return {min, max};
}