#ifndef EFTDEM_MOBILEMAPPINGREADER_H
#define EFTDEM_MOBILEMAPPINGREADER_H

#include "DataStructures.h"
#include "ICloudReader.h"
#include <vector>
#include <string>

/**
 * An ICloudReader that reads mobile mapping data. Expects 1 lines preceding the first data as well as 1 empty line at the
 * end.
 */
class MobileMappingReader : public ICloudReader {
private:
    void parseLineVector(std::vector<std::string> words, std::vector<doublePoint> *groundPoints, std::vector<doublePoint> *environmentPoints) override;

public:
    explicit MobileMappingReader(const std::string& fileName) : ICloudReader(fileName, 1) {};
};


#endif //EFTDEM_MOBILEMAPPINGREADER_H
