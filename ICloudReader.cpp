#include "ICloudReader.h"
#include "Pipeline.h"
#include <fstream>
#include <iostream>
#include <thread>
#include <future>

/**
 * Constructs a new ICloudReader that will read from the fileName-provided point cloud. The file must be comma-
 * separated, have one irrelevant line before data and conform to the format [x],[y],[z],[ground point? -> 1; environment point? -> 0],[reflection intensity]
 * for MobileMapping data.
 * For GroundRadar data, 9 irrelevant lines are expected preceding the data and the format is [x],[y],[z].
 * In both cases, one trailing empty line is expected.
 * @param fileName The path to the point cloud to be read from
 */
ICloudReader::ICloudReader(const std::string &fileName, unsigned int irrelevantLines) {
    stageUsesGPU = false;
    ICloudReader::irrelevantLines = irrelevantLines;
    ICloudReader::fileName = fileName;
}

ICloudReader::~ICloudReader() noexcept = default;

/**
 * Reads the file at the position in variable fileName (if there is one) while ignoring the first irrelevantLines lines.
 * @return a Vector containing the Lines of the File as Strings
 */
std::vector<std::string> ICloudReader::readFile() {
    std::fstream pointFile (fileName, std::ios::in);
    if (!pointFile.is_open()) {
        std::cout << "Specified file could not be opened." << std::endl;
        exit(Pipeline::EXIT_IO_ERROR);
    }

    std::vector<std::string> lines;
    std::string currentLine;
    for (auto i = 0u; i < irrelevantLines; i++) {
        getline(pointFile, currentLine);
    }
    while (getline(pointFile, currentLine)) {
        lines.emplace_back(currentLine);
    }

    pointFile.close();

    return lines;
}

/**
 * This extracts a doublePoint (i.e., an x, y and z coordinate as well as an intensity) from each line of lines in the
 * interval [begin, end).
 * @param lines The vector of lines to parse
 * @param groundPoints The vector to store the resulting doublePoints
 * @param begin The index of the first line, inclusive
 * @param end The index of the last line, exclusive (i.e., the index of the first line to not be parsed)
 * @return A pair of doublePoints where <return>.first is a doublePoint containing the minimum x,y,z and intensity values
 *      and <return>.second is a doublePoint containing the maximum x,y,z and intensity values
 */
std::pair<doublePoint, doublePoint> ICloudReader::parseFileContents(std::vector<std::string> *lines, std::vector<doublePoint> *groundPoints, std::vector<doublePoint> *environmentPoints, unsigned long begin = 0, unsigned long end = -1) {
    for (auto i = begin; i < end; i++) {
        std::vector<std::string> words(5);
        auto lastPosition = (unsigned long long) -1;
        for (auto & word : words) {
            ++lastPosition;
            auto delimiterPosition = lines->at(i).find(',', lastPosition);
            word = lines->at(i).substr(lastPosition, std::min(delimiterPosition - lastPosition, lines->at(i).size() - lastPosition));
            lastPosition = delimiterPosition;
        }

        parseLineVector(words, groundPoints, environmentPoints);
    }

    return mergeDoublePoints(*groundPoints);
}

/**
 * Reads the point cloud provided by fileName and return the parsed rawPointCloud containing all 3d points.
 * Parsing of the point cloud is done in a multi-threaded fashion.
 * @param generateOutput Whether or not to create a rawPointCloud. Should be true
 * @return The complete rawPointCloud
 */
rawPointCloud * ICloudReader::apply(bool generateOutput) {
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
        futuresVector.at(i) = std::async(&ICloudReader::parseFileContents, this, &lines, &(groundPointsVector->at(i)), &(environmentPointsVector->at(i)), batchSize * i, batchSize * (i+1));
    }
    futuresVector.at(numThreads - 1) = std::async(&ICloudReader::parseFileContents, this, &lines, &(groundPointsVector->at(numThreads - 1)), &(environmentPointsVector->at(numThreads - 1)), batchSize * (numThreads - 1), (unsigned long) lines.size());

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