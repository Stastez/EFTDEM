#ifndef EFTDEM_CLOUDREADER_H
#define EFTDEM_CLOUDREADER_H

#include <vector>
#include <string>
#include "DataStructures.h"
#include "ICloudReader.h"

class CloudReader : public ICloudReader {
    rawPointCloud apply(const std::string &fileName) override;
    void cleanUp() override;
};


#endif //EFTDEM_CLOUDREADER_H
