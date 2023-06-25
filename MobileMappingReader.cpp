#include "MobileMappingReader.h"
#include "Pipeline.h"
#include <fstream>
#include <iostream>
#include <thread>
#include <future>

/**
 * Constructs a new MobileMappingReader that will read from the fileName-provided point cloud. The file must be comma-
 * separated and conform to the format [x],[y],[z],[ground point? -> 1; environment point? -> 0],[reflection intensity]
 * @param fileName The path to the point cloud to be read from
 */
MobileMappingReader::MobileMappingReader(const std::string& fileName) {
    stageUsesGPU = false;
    MobileMappingReader::fileName = fileName;
}

void MobileMappingReader::cleanUp() {

}

std::vector<std::string> MobileMappingReader::readFile() {
    std::fstream pointFile (fileName, std::ios::in);
    if (!pointFile.is_open()) {
        std::cout << "Specified file could not be opened." << std::endl;
        exit(Pipeline::EXIT_IO_ERROR);
    }

    std::vector<std::string> lines;
    std::string currentLine;
    getline(pointFile, currentLine);
    while (getline(pointFile, currentLine)) {
        lines.emplace_back(currentLine);
    }

    pointFile.close();

    return lines;
}

std::pair<point, point> MobileMappingReader::parseFileContents(std::vector<std::string> *lines, std::vector<point> *groundPoints, std::vector<point> *environmentPoints, unsigned long begin = 0, unsigned long end = -1) {
    point min = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),std::numeric_limits<double>::max(), std::numeric_limits<int>::max()};
    point max = {std::numeric_limits<double>::min(), std::numeric_limits<double>::min(),std::numeric_limits<double>::min(), std::numeric_limits<int>::min()};

    for (auto i = begin; i < end; i++) {
        std::string words[5];
        size_t lastPosition = -1;
        for (auto & word : words) {
            ++lastPosition;
            auto delimiterPosition = lines->at(i).find(',', lastPosition);
            word = lines->at(i).substr(lastPosition, std::min(delimiterPosition - lastPosition, lines->at(i).size() - 1 - lastPosition));
            lastPosition = delimiterPosition;
        }

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

            groundPoints->emplace_back(p);
        } else environmentPoints->emplace_back(p);

        //std::cout << "Point: " << p.x << ", " << p.y << ", " << p.z << " Min: " << min.x << ", " << min.y << ", " << min.z << " Max: " << max.x << ", " << max.y << ", " << max.z << std::endl;
    }

    return {min, max};
}

rawPointCloud MobileMappingReader::apply(bool generateOutput) {
    if (!generateOutput) return {};

    std::vector<point> groundPoints, environmentPoints;
    std::cout << "Reading point cloud..." << std::endl;

    auto numThreads = 16;
    auto lines = readFile();
    auto batchSize = lines.size() / numThreads;
    auto extremesVector = std::vector<std::pair<point, point>>(numThreads);
    auto futuresVector = std::vector<std::future<std::pair<point, point>>>(numThreads);

    std::vector<std::vector<point>> groundPointsVector(numThreads), environmentPointsVector(numThreads);

    for (auto i = 0; i < numThreads - 1; i++) {
        futuresVector[i] = std::async(&parseFileContents, &lines, &groundPointsVector[i], &environmentPointsVector[i], batchSize * i, batchSize * (i+1));
    }
    futuresVector[numThreads - 1] = std::async(&parseFileContents, &lines, &groundPointsVector[numThreads - 1], &environmentPointsVector[numThreads - 1], batchSize * (numThreads - 1), lines.size());

    for (auto i = 0; i < numThreads; i++) {
        extremesVector[i] = futuresVector[i].get();
    }

    std::vector<point> minVector, maxVector;
    for (auto extremes : extremesVector) {
        minVector.emplace_back(extremes.first);
        maxVector.emplace_back(extremes.second);
    }
    std::pair<point,point> extremes;
    extremes.first = mergePoints(minVector).first;
    extremes.second = mergePoints(maxVector).second;

    for (auto i = 0; i < numThreads; i++) {
        groundPoints.insert(groundPoints.end(), groundPointsVector[i].begin(), groundPointsVector[i].end());
        environmentPoints.insert(environmentPoints.end(), environmentPointsVector[i].begin(), environmentPointsVector[i].end());
    }

    if (groundPoints.empty()) {
        extremes.first = {0,0,0, 0}; extremes.second = {0,0,0, 0};
    }

    return {.groundPoints = groundPoints, .environmentPoints = environmentPoints, .min = extremes.first, .max = extremes.second, .numberOfPoints = static_cast<unsigned int>(groundPoints.size())};
}