#include "TiffPipeline.h"
#include "GTiffReader.h"

#include <utility>

TiffPipeline::TiffPipeline(std::string path){
    TiffPipeline::path = std::move(path);
}

heightMap * TiffPipeline::execute() {
    GTiffReader * reader = new GTiffReader(path);

    map = reader->apply(true);
    delete reader;

    return map;
}

heightMap * TiffPipeline::executeAfterReader(rawPointCloud *pointCloud) {
    (void) pointCloud;
    return map;
}

GLHandler * TiffPipeline::getGLHandler() { return glHandler;}
ICloudReader * TiffPipeline::getCloudReader() {return nullptr; }

IHeightMapWriter *TiffPipeline::getHeightMapWriter() {return nullptr;}
