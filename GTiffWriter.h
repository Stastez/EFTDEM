#ifndef EFTDEM_GTIFFWRITER_H
#define EFTDEM_GTIFFWRITER_H

#include "IHeightMapWriter.h"
#include <string>

class GTiffWriter : public IHeightMapWriter {
private:
    bool writeLowDepth;
    std::string compressionAlgorithm;

public:
    explicit GTiffWriter(bool writeLowDepth, const std::string &destinationDEM, bool betterCompression);
    ~GTiffWriter() noexcept override;
    void apply(const heightMap *map, bool generateOutput) override;
    void writeRGB(std::vector<std::vector<int>> data, int resolutionX, int resolutionY) override;
    void setDestinationDEM(const std::string& newDestinationDEM) override;
};


#endif //EFTDEM_GTIFFWRITER_H
