#ifndef EFTDEM_GTIFFWRITER_H
#define EFTDEM_GTIFFWRITER_H

#include <string>
#include "IHeightMapWriter.h"

class GTiffWriter : public IHeightMapWriter {
private:
    bool writeLowDepth;

public:
    explicit GTiffWriter(bool writeLowDepth);
    void cleanUp() override;
    void apply(const heightMap *map, const std::string& destinationDEM) override;
};


#endif //EFTDEM_GTIFFWRITER_H
