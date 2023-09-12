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

MobileMappingReader::~MobileMappingReader() noexcept = default;

std::vector<std::string> MobileMappingReader::readFile() {
    std::fstream pointFile (fileName, std::ios::in);
    if (!pointFile.is_open()) {
        std::cout << "Specified file could not be opened: " << fileName << std::endl;
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

std::pair<doublePoint, doublePoint> MobileMappingReader::parseFileContents(std::vector<std::string> *lines, std::vector<doublePoint> *groundPoints, std::vector<doublePoint> *environmentPoints, unsigned long begin = 0, unsigned long end = -1) {
    for (auto i = begin; i < end; i++) {
        std::string words[5];
        auto lastPosition = (unsigned long long) -1;
        for (auto & word : words) {
            ++lastPosition;
            auto delimiterPosition = lines->at(i).find(',', lastPosition);
            word = lines->at(i).substr(lastPosition, std::min(delimiterPosition - lastPosition, lines->at(i).size() - lastPosition));
            lastPosition = delimiterPosition;
        }

        doublePoint p = {.x=stod(words[0]), .y=stod(words[1]), .z=stod(words[2]), .intensity=stoi(words[4])};

        if (words[3] == "1")
            groundPoints->emplace_back(p);
        else
            environmentPoints->emplace_back(p);
    }

    return mergeDoublePoints(*groundPoints);
}

rawPointCloud * MobileMappingReader::apply(bool generateOutput) {
    if (!generateOutput) return {};

    auto *groundPoints = new std::vector<doublePoint>(),
        *environmentPoints = new std::vector<doublePoint>();
    std::cout << "Reading point cloud..." << std::endl;

    auto numThreads = 16u;
    auto lines = readFile();
    auto batchSize = (unsigned long) (lines.size() / numThreads);
    auto extremesVector = std::vector<std::pair<doublePoint, doublePoint>>(numThreads);
    auto futuresVector = std::vector<std::future<std::pair<doublePoint, doublePoint>>>(numThreads);

    auto groundPointsVector = new std::vector<std::vector<doublePoint>>(numThreads),
        environmentPointsVector = new std::vector<std::vector<doublePoint>>(numThreads);

    for (auto i = 0u; i < numThreads - 1u; i++) {
        futuresVector.at(i) = std::async(&parseFileContents, &lines, &(groundPointsVector->at(i)), &(environmentPointsVector->at(i)), batchSize * i, batchSize * (i+1));
    }
    futuresVector.at(numThreads - 1) = std::async(&parseFileContents, &lines, &(groundPointsVector->at(numThreads - 1)), &(environmentPointsVector->at(numThreads - 1)), batchSize * (numThreads - 1), (unsigned long) lines.size());

    for (auto i = 0u; i < numThreads; i++) {
        extremesVector.at(i) = futuresVector.at(i).get();
    }

    std::vector<doublePoint> minVector, maxVector;
    for (auto extremes : extremesVector) {
        minVector.emplace_back(extremes.first);
        maxVector.emplace_back(extremes.second);
    }
    std::pair<doublePoint,doublePoint> extremes;
    extremes.first = mergeDoublePoints(minVector).first;
    extremes.second = mergeDoublePoints(maxVector).second;

    for (auto i = 0u; i < numThreads; i++) {
        groundPoints->insert(groundPoints->end(), groundPointsVector->at(i).begin(), groundPointsVector->at(i).end());
        environmentPoints->insert(environmentPoints->end(), environmentPointsVector->at(i).begin(), environmentPointsVector->at(i).end());
    }

    delete groundPointsVector;
    delete environmentPointsVector;

    if (groundPoints->empty()) {
        extremes.first = {0,0,0, 0}; extremes.second = {0,0,0, 0};
    }

    auto returnCloud = new rawPointCloud{.groundPoints = *groundPoints, .environmentPoints = *environmentPoints, .min = extremes.first, .max = extremes.second, .numberOfPoints = static_cast<unsigned int>(groundPoints->size())};

    delete environmentPoints;
    delete groundPoints;

    return returnCloud;
}