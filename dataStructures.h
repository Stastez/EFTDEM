
#ifndef EFTDEM_DATASTRUCTURES_H
#define EFTDEM_DATASTRUCTURES_H

#include <vector>

struct point {
    double x, y, z;
    int intensity;
};

struct rawPointCloud {
    std::vector<point> *groundPoints, *environmentPoints;
    double minX, maxX, minZ, maxZ;
};

struct pointGrid {
    std::vector<point> *points;
    unsigned long resolutionX, resolutionZ;
};

std::vector<point> get(pointGrid *g, unsigned long x, unsigned long z);
void set(pointGrid *g, unsigned long x, unsigned long z, std::vector<point> value);
void add(pointGrid *g, unsigned long x, unsigned long z, point value);
unsigned long long calculate2DCoordinates(pointGrid *g, unsigned long x, unsigned long z);

struct heightMap {
    double *heights;
    unsigned long resolutionX, resolutionZ;
};

#endif //EFTDEM_DATASTRUCTURES_H
