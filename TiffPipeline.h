#ifndef EFTDEM_TIFFPIPELINE_H
#define EFTDEM_TIFFPIPELINE_H

#include "IPipeline.h"
#include "ICloudSorter.h"
#include "IHeightMapWriter.h"
#include "ICloudReader.h"

class TiffPipeline : public IPipeline{
private:
    denormalizedHeightMap * map;
    std::string path;
public:
    explicit TiffPipeline(const std::string& tiffPath, const std::string& shaderDirectory);
    explicit TiffPipeline(const std::string& tiffPath, GLHandler *glHandler);

    heightMap * execute() override;
    heightMap * executeAfterReader(rawPointCloud *pointCloud) override;

    GLHandler * getGLHandler() override;
    ICloudReader * getCloudReader() override;
    ICloudSorter * getCloudSorter() override;
    IHeightMapWriter * getHeightMapWriter() override;
};


#endif //EFTDEM_TIFFPIPELINE_H
