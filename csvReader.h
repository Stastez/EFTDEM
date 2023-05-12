
#ifndef EFTDEM_CSVREADER_H
#define EFTDEM_CSVREADER_H

#include <vector>
#include <string>
#include "dataStructures.h"

class csvReader {
private:
    std::vector<point> groundPoints, environmentPoints;

public:
    rawPointCloud readCSV(const std::string& fileName);
};


#endif //EFTDEM_CSVREADER_H
