#ifndef EFTDEM_CLOUDREADER_H
#define EFTDEM_CLOUDREADER_H

#include "DataStructures.h"
#include "ICloudReader.h"
#include <vector>
#include <string>

class CloudReader : public ICloudReader {
private:
    //std::stringstream readFile();
    //void parseFileContents(std::stringstream fileContents, rawPointCloud *);

public:
    explicit CloudReader(const std::string& fileName);
    rawPointCloud apply(bool generateOutput) override;
    void cleanUp() override;
    std::vector<std::string> readFile();
    static std::pair<point, point> parseFileContents(std::vector<std::string> *lines, std::vector<point> *groundPoints, std::vector<point> *environmentPoints, unsigned long begin, unsigned long end);
};


#endif //EFTDEM_CLOUDREADER_H
