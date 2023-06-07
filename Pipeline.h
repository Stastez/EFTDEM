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
private:
    std::string sourceFilePath;
    std::string destinationPath;
    unsigned long pixelPerUnit;

    ICloudReader *reader{};
    ICloudSorter *sorter{};
    ICloudRasterizer *rasterizer{};
    IHeightMapFiller *filler{};
    IHeightMapWriter *writer{};
    GLHandler *glHandler{};

public:
    Pipeline(std::string sourceFilePath, std::string destinationPath, unsigned long pixelPerUnit);
    Pipeline(std::string sourceFilePath, std::string destinationPath, unsigned long pixelPerUnit, ICloudReader *, ICloudSorter *, ICloudRasterizer *, IHeightMapFiller *, IHeightMapWriter *);
    Pipeline(std::string sourceFilePath, std::string destinationPath, unsigned long pixelPerUnit, ICloudReader *, ICloudSorter *, ICloudRasterizer *, IHeightMapFiller *, IHeightMapWriter *, GLHandler *);
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
};


#endif //EFTDEM_PIPELINE_H
