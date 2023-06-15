#ifndef EFTDEM_CLOUDREADER_H
#define EFTDEM_CLOUDREADER_H

#include <vector>
#include <string>
#include "DataStructures.h"
#include "ICloudReader.h"

class CloudReader : public ICloudReader {
public:
    explicit CloudReader(const std::string& fileName);
    rawPointCloud apply(bool generateOutput) override;
    void cleanUp() override;
};


#endif //EFTDEM_CLOUDREADER_H
