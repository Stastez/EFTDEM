#include <fstream>
#include <iostream>
#include <sstream>
#include "CloudReader.h"
#include "Pipeline.h"

/**
 * Constructs a new CloudReader that will read from the fileName-provided point cloud. The file must be comma-
 * separated and conform to the format [x],[y],[z],[ground point? -> 1; environment point? -> 0],[reflection intensity]
 * @param fileName The path to the point cloud to be read from
 */
CloudReader::CloudReader(const std::string& fileName) {
    stageUsesGPU = false;
    CloudReader::fileName = fileName;
}

void CloudReader::cleanUp() {

}

rawPointCloud CloudReader::apply(bool generateOutput) {
    if (!generateOutput) return {};

    std::fstream pointFile (fileName, std::ios::in);
    if (!pointFile.is_open()) {
        std::cout << "Specified file could not be opened." << std::endl;
        exit(Pipeline::EXIT_IO_ERROR);
    }

    std::vector<point> groundPoints, environmentPoints;
    std::cout << "Reading point cloud..." << std::endl;

    point min = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),std::numeric_limits<double>::max(), std::numeric_limits<int>::max()};
    point max = {std::numeric_limits<double>::min(), std::numeric_limits<double>::min(),std::numeric_limits<double>::min(), std::numeric_limits<int>::min()};

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
            min.intensity = std::min(min.intensity, p.intensity);
            max.x = std::max(max.x, p.x);
            max.y = std::max(max.y, p.y);
            max.z = std::max(max.z, p.z);
            max.intensity = std::max(max.intensity, p.intensity);

            groundPoints.push_back(p);
        } else environmentPoints.push_back(p);
    }

    pointFile.close();

    if (groundPoints.empty()) {
        min = {0,0,0, 0}; max = {0,0,0, 0};
    }

    return {.groundPoints = groundPoints, .environmentPoints = environmentPoints, .min = min, .max = max, .numberOfPoints = static_cast<unsigned int>(groundPoints.size())};
}