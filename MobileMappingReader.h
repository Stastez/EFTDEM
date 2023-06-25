#ifndef EFTDEM_MOBILEMAPPINGREADER_H
#define EFTDEM_MOBILEMAPPINGREADER_H

#include "DataStructures.h"
#include "ICloudReader.h"
#include <vector>
#include <string>

class MobileMappingReader : public ICloudReader {
private:
    std::vector<std::string> readFile();
    static std::pair<point, point> parseFileContents(std::vector<std::string> *lines, std::vector<point> *groundPoints, std::vector<point> *environmentPoints, unsigned long begin, unsigned long end);

public:
    explicit MobileMappingReader(const std::string& fileName);
    rawPointCloud apply(bool generateOutput) override;
    void cleanUp() override;
};


#endif //EFTDEM_MOBILEMAPPINGREADER_H
