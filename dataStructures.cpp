#include "dataStructures.h"
#include <utility>
#include <iostream>

/**
 * Calculate the 1D array coordinate associated with (x,y) in grid g
 * @param g The reference grid
 * @param x
 * @param y
 * @return
 */
unsigned long long calculate1DCoordinate(pointGrid *g, unsigned long x, unsigned long y) {
    return y * g->resolutionX + x;
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
        exit(2);
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
    return g->points[calculate1DCoordinate(g, x, y)];
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
    g->points[calculate1DCoordinate(g, x, y)] = std::move(value);
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
    g->points[calculate1DCoordinate(g, x, y)].push_back(value);
}
