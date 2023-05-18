
#ifndef EFTDEM_DATASTRUCTURES_H
#define EFTDEM_DATASTRUCTURES_H

#include <vector>

struct point {
    double x, y, z;
    int intensity;
};

struct rawPointCloud {
    std::vector<point> *groundPoints, *environmentPoints;
    double minX, maxX, minY, maxY;
};

struct pointGrid {
    std::vector<point> *points;
    unsigned long resolutionX, resolutionY;
};

std::vector<point> get(pointGrid *g, unsigned long x, unsigned long y);
void set(pointGrid *g, unsigned long x, unsigned long y, std::vector<point> value);
void add(pointGrid *g, unsigned long x, unsigned long y, point value);
unsigned long long calculate1DCoordinate(pointGrid *g, unsigned long x, unsigned long y);

struct heightMap {
    double *heights;
    unsigned long resolutionX, resolutionY;
};

#endif //EFTDEM_DATASTRUCTURES_H
