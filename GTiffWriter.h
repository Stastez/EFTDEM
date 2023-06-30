#ifndef EFTDEM_GTIFFWRITER_H
#define EFTDEM_GTIFFWRITER_H

#include "IHeightMapWriter.h"
#include <string>

class GTiffWriter : public IHeightMapWriter {
private:
    bool writeLowDepth;

public:
    explicit GTiffWriter(bool writeLowDepth, const std::string& destinationDEM);
    ~GTiffWriter() noexcept override;
    void apply(const heightMap *map, bool generateOutput) override;
    void setDestinationDEM(const std::string& destinationDEM);
};


#endif //EFTDEM_GTIFFWRITER_H
