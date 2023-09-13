#ifndef EFTDEM_ICLOUDREADER_H
#define EFTDEM_ICLOUDREADER_H

#include "IPipelineComponent.h"
#include <string>

/**
 * The first stage of a pipeline. This takes some point cloud on disk and reads it into a rawPointCloud.
 */
class ICloudReader : public IPipelineComponent {
protected:
    std::string fileName;
    unsigned int irrelevantLines;
    virtual std::vector<std::string> readFile();
    /**
     * Converts the given vector of strings into a doublePoint and sorts it into groundPoints or environmentPoints
     * according to its segmentation.
     * @param words A vector of strings, i.e. numbers as strings
     * @param groundPoints The vector supposed to contain ground points
     * @param environmentPoints The vector supposed to contain all other points, e.g. vegetation
     */
    virtual void parseLineVector(std::vector<std::string> words, std::vector<doublePoint> *groundPoints, std::vector<doublePoint> *environmentPoints) = 0;
    std::pair<doublePoint, doublePoint> parseFileContents(std::vector<std::string> *lines, std::vector<doublePoint> *groundPoints, std::vector<doublePoint> *environmentPoints, unsigned long begin, unsigned long end);

public:
    explicit ICloudReader(const std::string &fileName, unsigned int irrelevantLines);
    ~ICloudReader() noexcept override;
    rawPointCloud *apply(bool generateOutput);
};



#endif //EFTDEM_ICLOUDREADER_H
