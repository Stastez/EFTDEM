
#ifndef EFTDEM_DATASTRUCTURES_H
#define EFTDEM_DATASTRUCTURES_H

#include <vector>

struct IPipelineComponentReturn {};

struct point {
    double x, y, z;
    int intensity;
};

struct rawPointCloud : IPipelineComponentReturn {
    std::vector<point> groundPoints;
    std::vector<point> environmentPoints;
    point min, max;
    unsigned int numberOfPoints;
};

struct pointGrid : IPipelineComponentReturn {
    std::vector<std::vector<point>> points;
    unsigned long resolutionX, resolutionY;
    point min, max;
    unsigned int numberOfPoints;
};

std::vector<point> get(pointGrid *g, unsigned long x, unsigned long y);
void set(pointGrid *g, unsigned long x, unsigned long y, std::vector<point> value);
void add(pointGrid *g, unsigned long x, unsigned long y, point value);
unsigned long long calculate1DCoordinate(pointGrid *g, unsigned long x, unsigned long y);

struct heightMap : IPipelineComponentReturn {
    std::vector<double> heights;
    unsigned long resolutionX, resolutionY;
    long long dataSize;
    point min, max;
};

heightMap emptyHeightMapfromHeightMap(heightMap *originalMap);
heightMap emptyHeightMapfromPointGrid(pointGrid *grid);

double denormalizeValue(double value, double min, double max);
double normalizeValue(double value, double min, double max);

#endif //EFTDEM_DATASTRUCTURES_H
