#ifndef EFTDEM_IPIPELINE_H
#define EFTDEM_IPIPELINE_H

#include "DataStructures.h"
#include "GLHandler.h"
#include "ICloudReader.h"
#include "ICloudSorter.h"
#include "IHeightMapWriter.h"

/**
 * A wrapper around pipelines to enable usage of different pipeline types in comparisons.
 */
class IPipeline {
protected:
    GLHandler *glHandler{};
public:
    virtual heightMap * execute() = 0;
    virtual heightMap * executeAfterReader(rawPointCloud *pointCloud) = 0;

    virtual GLHandler * getGLHandler() = 0;
    virtual ICloudReader * getCloudReader() = 0;
    virtual ICloudSorter * getCloudSorter() = 0;
    virtual IHeightMapWriter * getHeightMapWriter() = 0;

    virtual ~IPipeline() = default;
};


#endif //EFTDEM_IPIPELINE_H
