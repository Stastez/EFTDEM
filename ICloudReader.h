#ifndef EFTDEM_ICLOUDREADER_H
#define EFTDEM_ICLOUDREADER_H

#include "IPipelineComponent.h"
#include <string>

class ICloudReader : public IPipelineComponent {
protected:
    /**
     * The path of the point cloud to be read from.
     */
    std::string fileName;
    unsigned int irrelevantLines;
    virtual std::vector<std::string> readFile();

    virtual void parseLineVector(std::vector<std::string> words, std::vector<doublePoint> *groundPoints, std::vector<doublePoint> *environmentPoints) = 0;

    std::pair<doublePoint, doublePoint> parseFileContents(std::vector<std::string> *lines, std::vector<doublePoint> *groundPoints, std::vector<doublePoint> *environmentPoints, unsigned long begin, unsigned long end);

public:
    explicit ICloudReader(const std::string &fileName, unsigned int irrelevantLines);
    ~ICloudReader() noexcept override;
    /**
     * Use the currently attached ICloudReader to read the previously specified point cloud from disk.
     * @param generateOutput Whether to return a fully formed rawPointCloud (including the actual points). This may be set to false if the following stage
     *      uses OpenGL and can employ the already set buffers on the GPU
     * @return A fully formed rawPointCloud if generateOutput is true, a rawPointCloud only containing all necessary metadata otherwise
     */
    rawPointCloud *apply(bool generateOutput);
};



#endif //EFTDEM_ICLOUDREADER_H
