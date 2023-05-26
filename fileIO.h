
#ifndef EFTDEM_FILEIO_H
#define EFTDEM_FILEIO_H

#include <vector>
#include <string>
#include "dataStructures.h"

class fileIO {
public:
    static rawPointCloud readCSV(const std::string& fileName);
    static void writeTIFF(const heightMap *map, const std::string& destinationDEM, bool writeLowDepth = true);
};


#endif //EFTDEM_FILEIO_H
