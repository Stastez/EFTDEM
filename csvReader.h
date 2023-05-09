
#ifndef EFTDEM_CSVREADER_H
#define EFTDEM_CSVREADER_H

#include <vector>
#include <string>
#include "dataStructures.h"

class csvReader {
private:
    std::vector<point> groundPoints, environmentPoints;

public:
    std::pair<std::vector<point>*, std::vector<point>*> readCSV(const std::string& fileName);
};


#endif //EFTDEM_CSVREADER_H
