#ifndef EFTDEM_GTIFFREADER_H
#define EFTDEM_GTIFFREADER_H

#include "DataStructures.h"
#include <string>
#include "IHeightMapReader.h"

class GTiffReader : public IHeightMapReader {
public:
    explicit GTiffReader(const std::string& path);
    ~GTiffReader() noexcept override;
    heightMap * apply(bool generateOutput) override;
    void setSourceDEM(const std::string& sourceDEM);
};

#endif //EFTDEM_GTIFFREADER_H
