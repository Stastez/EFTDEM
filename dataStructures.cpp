#include "dataStructures.h"
#include <utility>
#include <iostream>

/**
 * Calculate the 1D array coordinate associated with (x,z) in grid g
 * @param g The reference grid
 * @param x
 * @param z
 * @return
 */
unsigned long long calculate2DCoordinates(pointGrid *g, unsigned long x, unsigned long z) {
    return z * g->resolutionX + x;
}

/**
 * Check whether the given (x,z) is outside of the associated grid
 * @param g The reference grid
 * @param x
 * @param z
 */
void validateCoordinates(pointGrid *g, unsigned long x, unsigned long z) {
    if (x > g->resolutionX - 1 || z > g->resolutionZ - 1) {
        std::cout << "Given coordinates outside of grid! Grid dimensions: (" << g->resolutionX << ", " << g->resolutionZ << "), given coordinates: (" << x << ", " << z << ")";
        exit(2);
    }
}

/**
 * Returns the vector situated at posiiton (x,z) of grid g
 * @param g The pointGrid to be accessed
 * @param x The x coordinate of the requested position (x in [0, g->resolutionX - 1])
 * @param z The z coordinate of the requested position (z in [0, g->resolutionZ - 1])
 * @return
 */
std::vector<point> get(pointGrid *g, unsigned long x, unsigned long z){
    validateCoordinates(g, x, z);
    return g->points[calculate2DCoordinates(g, x, z)];
}

/**
 * Replaces the vector situated at position (x,z) of grid g with value
 * @param g The pointGrid to be updated
 * @param x The x coordinate of the requested position (x in [0, g->resolutionX - 1])
 * @param z The z coordinate of the requested position (z in [0, g->resolutionZ - 1])
 * @param value The replacement vector
 */
void set(pointGrid *g, unsigned long x, unsigned long z, std::vector<point> value){
    validateCoordinates(g, x, z);
    g->points[calculate2DCoordinates(g, x, z)] = std::move(value);
}

/**
 * Appends the given point to the vector situated at position (x,z) of grid g
 * @param g The pointGrid to be updated
 * @param x The x coordinate of the requested position (x in [0, g->resolutionX - 1])
 * @param z The z coordinate of the requested position (z in [0, g->resolutionZ - 1])
 * @param value The point to be appended
 */
void add(pointGrid *g, unsigned long x, unsigned long z, point value){
    validateCoordinates(g, x, z);
    g->points[calculate2DCoordinates(g, x, z)].push_back(value);
}
