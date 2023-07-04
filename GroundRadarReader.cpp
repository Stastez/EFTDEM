#include "GroundRadarReader.h"
#include "Pipeline.h"
#include <fstream>
#include <iostream>
#include <thread>
#include <future>

GroundRadarReader::GroundRadarReader(const std::string& fileName) {
    stageUsesGPU = false;
    GroundRadarReader::fileName = fileName;
}

GroundRadarReader::~GroundRadarReader() noexcept = default;

std::vector<std::string> GroundRadarReader::readFile() {
    std::fstream pointFile (fileName, std::ios::in);
    if (!pointFile.is_open()) {
        std::cout << "Specified file could not be opened." << std::endl;
        exit(Pipeline::EXIT_IO_ERROR);
    }

    std::vector<std::string> lines;
    std::string currentLine;
    for (int i = 0; i < 9; i++) {
        getline(pointFile, currentLine);
    }
    while (getline(pointFile, currentLine)) {
        lines.emplace_back(currentLine);
    }

    pointFile.close();

    return lines;
}

std::pair<point, point> GroundRadarReader::parseFileContents(std::vector<std::string> *lines, std::vector<point> *groundPoints, unsigned long begin = 0, unsigned long end = -1) {
    point min = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),std::numeric_limits<double>::max(), -1};
    point max = {-std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(),-std::numeric_limits<double>::max(), -1};

    for (auto i = begin; i < end; i++) {
        std::string words[3];
        auto lastPosition = (unsigned long long) -1;
        for (auto & word : words) {
            ++lastPosition;
            auto delimiterPosition = lines->at(i).find(',', lastPosition);
            word = lines->at(i).substr(lastPosition, std::min(delimiterPosition - lastPosition, lines->at(i).size() - lastPosition));
            lastPosition = delimiterPosition;
        }

        point p = {.x=stod(words[0]), .y=stod(words[1]), .z=stod(words[2]), .intensity=-1};

        min.x = std::min(min.x, p.x);
        min.y = std::min(min.y, p.y);
        min.z = std::min(min.z, p.z);
        max.x = std::max(max.x, p.x);
        max.y = std::max(max.y, p.y);
        max.z = std::max(max.z, p.z);

        groundPoints->emplace_back(p);

        //std::cout << "Point: " << p.x << ", " << p.y << ", " << p.z << " Min: " << min.x << ", " << min.y << ", " << min.z << " Max: " << max.x << ", " << max.y << ", " << max.z << std::endl;
    }

    return {min, max};
}

rawPointCloud * GroundRadarReader::apply(bool generateOutput) {
    if (!generateOutput) return {};

    auto groundPoints = new std::vector<point>();
    std::cout << "Reading point cloud..." << std::endl;

    auto numThreads = 16;
    auto lines = readFile();
    auto batchSize = lines.size() / numThreads;
    auto extremesVector = std::vector<std::pair<point, point>>(numThreads);
    auto futuresVector = std::vector<std::future<std::pair<point, point>>>(numThreads);

    auto groundPointsVector = new std::vector<std::vector<point>>(numThreads);

    for (auto i = 0; i < numThreads - 1; i++) {
        futuresVector.at(i) = std::async(&parseFileContents, &lines, &(groundPointsVector->at(i)), batchSize * i, batchSize * (i+1));
    }
    futuresVector.at(numThreads - 1) = std::async(&parseFileContents, &lines, &(groundPointsVector->at(numThreads - 1)), batchSize * (numThreads - 1), lines.size());

    for (auto i = 0; i < numThreads; i++) {
        extremesVector.at(i) = futuresVector.at(i).get();
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
        groundPoints->insert(groundPoints->end(), groundPointsVector->at(i).begin(), groundPointsVector->at(i).end());
    }

    delete groundPointsVector;

    if (groundPoints->empty()) {
        extremes.first = {0,0,0, 0}; extremes.second = {0,0,0, 0};
    }

    auto readerReturn = new rawPointCloud{.groundPoints = *groundPoints, .environmentPoints = std::vector<point>(0), .min = extremes.first, .max = extremes.second, .numberOfPoints = static_cast<unsigned int>(groundPoints->size())};
    delete groundPoints;
    return readerReturn;
}