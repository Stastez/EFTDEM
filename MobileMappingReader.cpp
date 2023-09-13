#include "MobileMappingReader.h"
#include "Pipeline.h"
#include <fstream>

void MobileMappingReader::parseLineVector(std::vector<std::string> words, std::vector<doublePoint> *groundPoints, std::vector<doublePoint> *environmentPoints) {
    doublePoint p = {.x=stod(words[0]), .y=stod(words[1]), .z=stod(words[2]), .intensity=stoi(words[4])};

    if (words[3] == "1")
        groundPoints->emplace_back(p);
    else
        environmentPoints->emplace_back(p);
}