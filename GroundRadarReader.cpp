#include "GroundRadarReader.h"
#include "Pipeline.h"
#include <fstream>

void GroundRadarReader::parseLineVector(std::vector<std::string> words, std::vector<doublePoint> *groundPoints, std::vector<doublePoint> *environmentPoints) {
    (void) environmentPoints;

    doublePoint p = {.x=stod(words[0]), .y=stod(words[1]), .z=stod(words[2]), .intensity=-1};

    groundPoints->emplace_back(p);
}