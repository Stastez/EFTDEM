#ifndef EFTDEM_TIFFPIPELINE_H
#define EFTDEM_TIFFPIPELINE_H

#include "IPipeline.h"

class TiffPipeline : public IPipeline{
private:
    heightMap * map;
    std::string path;
public:
    explicit TiffPipeline(std::string path);

    heightMap * execute() override;
    heightMap * executeAfterReader(rawPointCloud *pointCloud) override;

    GLHandler * getGLHandler() override;
    ICloudReader * getCloudReader() override;
};


#endif //EFTDEM_TIFFPIPELINE_H
