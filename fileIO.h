
#ifndef EFTDEM_FILEIO_H
#define EFTDEM_FILEIO_H

#include <vector>
#include <string>
#include "dataStructures.h"

class fileIO {
private:
    std::vector<point> groundPoints, environmentPoints;

public:
    rawPointCloud readCSV(const std::string& fileName);
    void writeTIFF(const heightMap *map, int resolutionX, int resolutionZ, bool writeLowDepth = false);
};


#endif //EFTDEM_FILEIO_H
