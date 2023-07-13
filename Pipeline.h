#ifndef EFTDEM_PIPELINE_H
#define EFTDEM_PIPELINE_H

#include "IPipeline.h"
#include "IPipelineComponent.h"
#include "ICloudReader.h"
#include "ICloudSorter.h"
#include "ICloudRasterizer.h"
#include "IHeightMapFiller.h"
#include "IHeightMapWriter.h"
#include "GLHandler.h"

class Pipeline : public IPipeline {
friend class ConfigProvider;

private:
    ICloudReader *reader{};
    ICloudSorter *sorter{};
    ICloudRasterizer *rasterizer{};
    IHeightMapFiller *filler{};
    IHeightMapWriter *writer{};
    GLHandler *glHandler{};

public:
    ~Pipeline() override;

    explicit Pipeline(GLHandler *glHandler);
    explicit Pipeline(const std::string& shaderDirectory);
    bool isOperable();

    void attachElements(ICloudReader *, ICloudSorter *, ICloudRasterizer *, IHeightMapFiller *, IHeightMapWriter *);

    heightMap * execute() override;
    heightMap * executeAfterReader(rawPointCloud *pointCloud) override;

    GLHandler * getGLHandler() override;
    ICloudReader * getCloudReader() override;
    IHeightMapWriter * getHeightMapWriter() override;

    enum exitCodes {
        EXIT_INVALID_COMMAND_LINE_ARGUMENTS = 101,
        EXIT_INVALID_FUNCTION_PARAMETERS,
        EXIT_IO_ERROR,
        EXIT_OPENGL_ERROR,
        EXIT_DEPENDENCY_ERROR,
        EXIT_INVALID_CONFIGURATION,
        EXIT_NOT_YET_IMPLEMENTED
    };
};


#endif //EFTDEM_PIPELINE_H
