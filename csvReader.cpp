
#include "csvReader.h"
#include <fstream>
#include <iostream>
#include <sstream>

using std::vector, std::pair;

pair<vector<point>*, vector<point>*> csvReader::readCSV(const std::string& fileName) {
    std::fstream pointFile (fileName, std::ios::in);
    if (!pointFile.is_open()) {
        std::cout << "Specified file could not be opened." << std::endl;
        return {&groundPoints, &environmentPoints};
    }
    std::string line;
    std::getline(pointFile, line);
    while(std::getline(pointFile, line)){
        std::stringstream str(line);

        std::string words[5];
        for (auto & word : words) std::getline(str, word, ',');

        point p = {.x=stod(words[0]), .y=stod(words[1]), .z=stod(words[2]), .intensity=stoi(words[4])};
        if (words[3] == "1") groundPoints.push_back(p);
        else environmentPoints.push_back(p);
    }

    pointFile.close();
    return {&groundPoints, &environmentPoints};
}
