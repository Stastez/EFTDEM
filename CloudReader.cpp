#include <fstream>
#include <iostream>
#include <sstream>
#include "CloudReader.h"

/**
 * Reads the content of the fileName-provided file into a raw, non-grid point cloud struct. The file must be comma-
 * separated and conform to the format [x],[y],[z],[ground point? -> 1; environment point? -> 0],[reflection intensity]
 * @param fileName The path to the csv containing the point cloud
 * @return A new rawPointCloud struct
 */
rawPointCloud CloudReader::apply(const std::string& fileName) {
    std::fstream pointFile (fileName, std::ios::in);
    if (!pointFile.is_open()) {
        std::cout << "Specified file could not be opened." << std::endl;
        exit(3);
    }

    std::vector<point> groundPoints, environmentPoints;
    std::cout << "Reading point cloud..." << std::endl;

    point min = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),std::numeric_limits<double>::max()};
    point max = {std::numeric_limits<double>::min(), std::numeric_limits<double>::min(),std::numeric_limits<double>::min()};

    std::string line;
    std::getline(pointFile, line);
    while(std::getline(pointFile, line)){
        std::stringstream str(line);

        std::string words[5];
        for (auto & word : words) std::getline(str, word, ',');

        point p = {.x=stod(words[0]), .y=stod(words[1]), .z=stod(words[2]), .intensity=stoi(words[4])};

        if (words[3] == "1") {
            min.x = std::min(min.x, p.x);
            min.y = std::min(min.y, p.y);
            min.z = std::min(min.z, p.z);
            max.x = std::max(max.x, p.x);
            max.y = std::max(max.y, p.y);
            max.z = std::max(max.z, p.z);

            groundPoints.push_back(p);
        } else environmentPoints.push_back(p);
    }

    pointFile.close();

    if (groundPoints.empty()) {
        min = {0,0,0}; max = {0,0,0};
    }

    return {.groundPoints = groundPoints, .environmentPoints = environmentPoints, .min = min, .max = max};
}