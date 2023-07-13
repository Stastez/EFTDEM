#ifndef EFTDEM_TIFFPIPELINE_H
#define EFTDEM_TIFFPIPELINE_H

#include "IPipeline.h"

class TiffPipeline : public IPipeline{
private:
    denormalizedHeightMap * map;
    std::string path;
public:
    explicit TiffPipeline(std::string path);

    heightMap * execute() override;
    heightMap * executeAfterReader(rawPointCloud *pointCloud) override;

    GLHandler * getGLHandler() override;
    ICloudReader * getCloudReader() override;
    IHeightMapWriter * getHeightMapWriter() override;
};


#endif //EFTDEM_TIFFPIPELINE_H
