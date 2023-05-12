
#include "csvReader.h"
#include <fstream>
#include <iostream>
#include <sstream>

using std::vector, std::pair;

rawPointCloud csvReader::readCSV(const std::string& fileName) {
    std::fstream pointFile (fileName, std::ios::in);
    if (!pointFile.is_open()) {
        std::cout << "Specified file could not be opened." << std::endl;
        exit(3);
        //return {&groundPoints, &environmentPoints, 0, 0, 0, 0};
    }

    double minX = std::numeric_limits<double>::max(), minZ = std::numeric_limits<double>::max(), maxX = std::numeric_limits<double>::min(), maxZ = std::numeric_limits<double>::min();

    std::string line;
    std::getline(pointFile, line);
    while(std::getline(pointFile, line)){
        std::stringstream str(line);

        std::string words[5];
        for (auto & word : words) std::getline(str, word, ',');

        point p = {.x=stod(words[0]), .y=stod(words[1]), .z=stod(words[2]), .intensity=stoi(words[4])};

        minX = std::min(minX, p.x); minZ = std::min(minZ, p.z); maxX = std::max(maxX, p.x); maxZ = std::max(maxZ, p.z);

        if (words[3] == "1") groundPoints.push_back(p);
        else environmentPoints.push_back(p);
    }

    pointFile.close();

    if (groundPoints.empty()) {
        minX = 0; maxX = 0; minZ = 0; maxZ = 0;
    }

    return {&groundPoints, &environmentPoints, minX, maxX, minZ, maxZ};
}
