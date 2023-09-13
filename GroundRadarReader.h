#ifndef EFTDEM_GROUNDRADARREADER_H
#define EFTDEM_GROUNDRADARREADER_H

#include "ICloudReader.h"

/**
 * An ICloudReader that reads ground radar data. Expects 9 lines preceding the first data as well as 1 empty line at the
 * end.
 */
class GroundRadarReader : public ICloudReader {
private:
    void parseLineVector(std::vector<std::string> words, std::vector<doublePoint> *groundPoints, std::vector<doublePoint> *environmentPoints) override;

public:
    explicit GroundRadarReader(const std::string& fileName) : ICloudReader(fileName, 9) {};
};


#endif //EFTDEM_GROUNDRADARREADER_H
