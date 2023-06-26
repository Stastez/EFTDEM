#ifndef EFTDEM_GROUNDRADARREADER_H
#define EFTDEM_GROUNDRADARREADER_H

#include "ICloudReader.h"

class GroundRadarReader : public ICloudReader {
private:
    std::vector<std::string> readFile();
    static std::pair<point, point> parseFileContents(std::vector<std::string> *lines, std::vector<point> *groundPoints, unsigned long begin, unsigned long end);

public:
    explicit GroundRadarReader(const std::string& fileName);
    ~GroundRadarReader() noexcept override;
    rawPointCloud apply(bool generateOutput) override;
};


#endif //EFTDEM_GROUNDRADARREADER_H
