
#ifndef EFTDEM_DATASTRUCTURES_H
#define EFTDEM_DATASTRUCTURES_H

#include <vector>
#include <ostream>

struct doublePoint {
    double x, y, z;
    int intensity;
};

struct floatPoint {
    float x, y, z;
    int intensity;
};

std::ostream &operator<<(std::ostream &os, doublePoint const &point);
std::ostream &operator<<(std::ostream &os, floatPoint const &point);

struct rawPointCloud {
    std::vector<doublePoint> groundPoints;
    std::vector<doublePoint> environmentPoints;
    doublePoint min, max;
    unsigned int numberOfPoints;
};

struct pointGrid {
    std::vector<std::vector<floatPoint>> points;
    unsigned long resolutionX, resolutionY;
    doublePoint min, max;
    unsigned int numberOfPoints;
};

std::vector<floatPoint> get(pointGrid *g, unsigned long x, unsigned long y);
void set(pointGrid *g, unsigned long x, unsigned long y, std::vector<floatPoint> value);
void add(pointGrid *g, unsigned long x, unsigned long y, floatPoint value);
unsigned long long calculate1DCoordinate(const pointGrid *g, unsigned long x, unsigned long y);

struct heightMap {
    std::vector<float> heights;
    unsigned long resolutionX, resolutionY;
    long dataSize;
    doublePoint min, max;
};

struct denormalizedHeightMap {
    std::vector<double> heights;
    unsigned long resolutionX, resolutionY;
    long dataSize;
    doublePoint min, max;
};

heightMap * emptyHeightMapFromHeightMap(heightMap *map);
heightMap * emptyHeightMapFromPointGrid(pointGrid *grid);
rawPointCloud * emptyPointCloudFromHeightMap(heightMap *map);

unsigned long long calculate1DCoordinate(const heightMap *h, unsigned long x, unsigned long y);

double denormalizeValue(float value, double min, double max);
float normalizeValue(double value, double min, double max);
floatPoint normalizeValue(doublePoint value, doublePoint min, doublePoint max);

[[maybe_unused]] doublePoint denormalizeValue(floatPoint value, doublePoint min, doublePoint max);

std::pair<doublePoint, doublePoint> mergeDoublePoints(doublePoint p1, doublePoint p2);
std::pair<doublePoint, doublePoint> mergeDoublePoints(const std::vector<doublePoint>& points);
std::pair<floatPoint, floatPoint> mergeFloatPoints(floatPoint p1, floatPoint p2);
std::pair<floatPoint, floatPoint> mergeFloatPoints(const std::vector<floatPoint>& points);

#endif //EFTDEM_DATASTRUCTURES_H
