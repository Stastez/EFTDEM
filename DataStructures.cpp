#include "DataStructures.h"
#include "Pipeline.h"
#include <utility>
#include <iostream>

/**
 * Converts a doublePoint to a printable stream.
 * @param os The stream to be printed on
 * @param point The doublePoint to be printed
 * @return A reference to the stream containing the original stream and the doublePoint
 */
std::ostream &operator<<(std::ostream &os, doublePoint const &point) {
    return os << "doublePoint :: x: " << point.x << ", y: " << point.y << ", z: " << point.z;
}

/**
 * Converts a floatPoint to a printable stream.
 * @param os The stream to be printed on
 * @param point The floatPoint to be printed
 * @return A reference to the stream containing the original stream and the floatPoint
 */
std::ostream &operator<<(std::ostream &os, floatPoint const &point) {
    return os << "floatPoint :: x: " << point.x << ", y: " << point.y << ", z: " << point.z;
}

/**
 * Calculate the 1D array coordinate associated with (x,y) in grid g
 * @param g The reference grid
 * @param x
 * @param y
 * @return The 1D representation of (x,y) in g
 */
unsigned long long calculate1DCoordinate(const pointGrid *g, unsigned long x, unsigned long y) {
    return y * g->resolutionX + x;
}

/**
 * Calculate the 1D array coordinate associated with (x,y) in heightMap h
 * @param h The reference heightMap
 * @param x
 * @param y
 * @return The 1D representation of (x,y) in h
 */
unsigned long long calculate1DCoordinate(const heightMap *h, unsigned long x, unsigned long y) {
    return y * h->resolutionX + x;
}

/**
 * Checks whether the given (x,y) is outside of the associated grid. If the given (x,y) is out of bounds, an exception
 * is thrown.
 * @param g The reference grid
 * @param x
 * @param y
 * @throws std::exception If the given (x,y) are out of bounds for g
 */
void validateCoordinates(pointGrid *g, unsigned long x, unsigned long y) {
    if (x > g->resolutionX - 1 || y > g->resolutionY - 1) {
        std::cout << "Given coordinates outside of grid! Grid dimensions: (" << g->resolutionX << ", " << g->resolutionY << "), given coordinates: (" << x << ", " << y << ")";
        throw std::exception();
    }
}

/**
 * Returns the vector situated at position (x,y) of grid g
 * @param g The pointGrid to be accessed
 * @param x The x coordinate of the requested position (x in [0, g->resolutionX - 1])
 * @param y The y coordinate of the requested position (y in [0, g->resolutionY - 1])
 * @throws std::exception If the given (x,y) are out of bounds for g
 * @return The floatPoint vector situated at (x,y) in g
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
 * @throws std::exception If the given (x,y) are out of bounds for g
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
 * @throws std::exception If the given (x,y) are out of bounds for g
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

/**
 * Normalizes the given doublePoint component-wise from the range of [min,max] to [0,1]
 * @param value The doublePoint to be normalized
 * @param min The doublePoint containing the minimum values for x,y,z
 * @param max The doublePoint containing the maximum values for x,y,z
 * @return A floatPoint with x,y,z in [0,1]
 */
floatPoint normalizeValue(doublePoint value, doublePoint min, doublePoint max) {
    return floatPoint{.x = normalizeValue(value.x, min.x, max.x),
                 .y = normalizeValue(value.y, min.y, max.y),
                 .z = normalizeValue(value.z, min.z, max.z),
                 .intensity = value.intensity};
}

/**
 * Denormalizes the given floatPoint component-wise from the range of [0,1] to [min,max].
 * @param value The floatPoint to be denormalized
 * @param min The doublePoint containing the minimum values for x,y,z
 * @param max The doublePoint containing the maximum values for x,y,z
 * @return A doublePoint with x,y,z in [min,max]
 */
doublePoint denormalizeValue(floatPoint value, doublePoint min, doublePoint max) {
    return doublePoint{.x = denormalizeValue(value.x, min.x, max.x),
            .y = denormalizeValue(value.y, min.y, max.y),
            .z = denormalizeValue(value.z, min.z, max.z),
            .intensity = value.intensity};
}

/**
 * Allocates a new, empty heightMap with the same metadata (e.g., resolution) as the given grid.
 * @param grid The pointGrid of which to copy the metadata
 * @return A pointer to an empty heightMap
 */
heightMap * emptyHeightMapFromPointGrid(pointGrid *grid) {
    return new heightMap{.heights = std::vector<float>(grid->resolutionX * grid->resolutionY),
            .resolutionX = grid->resolutionX,
            .resolutionY = grid->resolutionY,
            .dataSize = static_cast<long>(sizeof(float) * grid->resolutionX * grid->resolutionY),
            .min = grid->min,
            .max = grid->max};
}

/**
 * Allocates a new, empty heightMap with the same metadata (e.g., resolution) as the given map.
 * @param map The heightMap of which to copy the metadata
 * @return A pointer to an empty heightMap
 */
heightMap * emptyHeightMapFromHeightMap(heightMap *map){
    return new heightMap{.heights = std::vector<float>(map->resolutionX * map->resolutionY),
            .resolutionX = map->resolutionX,
            .resolutionY = map->resolutionY,
            .dataSize = map->dataSize,
            .min = map->min,
            .max = map->max };
}

/**
 * Allocates a new, empty rawPointCloud with the same metadata (i.e., min and max) as the given map.
 * @param map The heightMap of which to copy the metadata
 * @return A pointer to an empty rawPointCloud
 */
rawPointCloud * emptyPointCloudFromHeightMap(heightMap *map) {
    return new rawPointCloud{
            .groundPoints = std::vector<doublePoint>(),
            .environmentPoints = std::vector<doublePoint>(),
            .min = map->min, .max = map->max,
            .numberOfPoints = 0 };
}

/**
 * Calculates the absolute component-wise minimum point of p1 and p2 as well as the absolute component-wise maximum point.
 * @param p1 One doublePoint
 * @param p2 Another doublePoint
 * @return A pair containing 1) the absolute minimum point and 2) the absolute maximum point
 */
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

/**
 * Calculates the absolute component-wise minimum point of all points in points, as well as the absolute component-wise
 * maximum point.
 * @param points A vector of doublePoints
 * @return A pair containing 1) the absolute minimum point and 2) the absolute maximum point
 */
std::pair<doublePoint, doublePoint> mergeDoublePoints(const std::vector<doublePoint>& points) {
    auto min = doublePoint{std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<int>::max()};
    auto max = doublePoint{-std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(), std::numeric_limits<int>::min()};

    for (auto point : points) {
        min = mergeDoublePoints(min, point).first;
        max = mergeDoublePoints(max, point).second;
    }

    return {min, max};
}

/**
 * Calculates the absolute component-wise minimum point of p1 and p2 as well as the absolute component-wise maximum point.
 * @param p1 One floatPoint
 * @param p2 Another floatPoint
 * @return A pair containing 1) the absolute minimum point and 2) the absolute maximum point
 */
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

/**
 * Calculates the absolute component-wise minimum point of all points in points, as well as the absolute component-wise
 * maximum point.
 * @param points A vector of floatPoints
 * @return A pair containing 1) the absolute minimum point and 2) the absolute maximum point
 */
std::pair<floatPoint, floatPoint> mergeFloatPoints(const std::vector<floatPoint>& points) {
    auto min = floatPoint{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<int>::max()};
    auto max = floatPoint{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), std::numeric_limits<int>::min()};

    for (auto point : points) {
        min = mergeFloatPoints(min, point).first;
        max = mergeFloatPoints(max, point).second;
    }

    return {min, max};
}