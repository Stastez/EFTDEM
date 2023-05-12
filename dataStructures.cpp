#include "dataStructures.h"

#include <utility>

std::vector<point> get(pointGrid g, unsigned long x, unsigned long z){
    unsigned long coord = z*g.resolutionX + x;
    return g.points[coord];
}

void set(pointGrid g, unsigned long x, unsigned long z, std::vector<point> value){
    unsigned long coord = z*g.resolutionX + x;
    g.points[coord] = std::move(value);
}

void add(pointGrid g, unsigned long x, unsigned long z, point value){
    unsigned long coord = z*g.resolutionX + x;
    g.points[coord].push_back(value);
}
