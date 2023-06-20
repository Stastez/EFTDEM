#ifndef EFTDEM_PIPELINE_H
#define EFTDEM_PIPELINE_H

#include "IPipelineComponent.h"
#include "ICloudReader.h"
#include "ICloudSorter.h"
#include "ICloudRasterizer.h"
#include "IHeightMapFiller.h"
#include "IHeightMapWriter.h"
#include "GLHandler.h"

class Pipeline {
friend class ConfigProvider;

private:

    ICloudReader *reader{};
    ICloudSorter *sorter{};
    ICloudRasterizer *rasterizer{};
    IHeightMapFiller *filler{};
    IHeightMapWriter *writer{};
    GLHandler *glHandler{};

public:
    ~Pipeline();

    Pipeline();
    Pipeline(ICloudReader *, ICloudSorter *, ICloudRasterizer *, IHeightMapFiller *, IHeightMapWriter *);
    Pipeline(ICloudReader *, ICloudSorter *, ICloudRasterizer *, IHeightMapFiller *, IHeightMapWriter *, GLHandler *);
    bool isOperable();

    void attachElements(ICloudReader *, ICloudSorter *, ICloudRasterizer *, IHeightMapFiller *, IHeightMapWriter *);

    void execute();

    ICloudReader *getReader();
    void setReader(ICloudReader *reader);
    ICloudSorter *getSorter();
    void setSorter(ICloudSorter *sorter);
    ICloudRasterizer *getRasterizer();
    void setRasterizer(ICloudRasterizer *rasterizer);
    IHeightMapFiller *getFiller();
    void setFiller(IHeightMapFiller *filler);
    IHeightMapWriter *getWriter();
    void setWriter(IHeightMapWriter *writer);
    GLHandler *getGLHandler();
    void setGLHandler(GLHandler *glHandler);

    enum exitCodes {
        EXIT_REGULAR_EXECUTION,
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