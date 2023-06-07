#include "Pipeline.h"

#include <utility>

Pipeline::Pipeline(std::string sourceFilePath, std::string destinationPath, unsigned long pixelPerUnit) {
    this->sourceFilePath = std::move(sourceFilePath);
    this->destinationPath = std::move(destinationPath);
    this->pixelPerUnit = pixelPerUnit;
    this->glHandler = new GLHandler();
    this->glHandler->initializeGL(false);
}

Pipeline::Pipeline(std::string sourceFilePath, std::string destinationPath, unsigned long pixelPerUnit, ICloudReader *reader, ICloudSorter *sorter, ICloudRasterizer *rasterizer, IHeightMapFiller *filler, IHeightMapWriter *writer)
                    : Pipeline(std::move(sourceFilePath), std::move(destinationPath), pixelPerUnit, reader, sorter, rasterizer, filler, writer, new GLHandler()) {}

Pipeline::Pipeline(std::string sourceFilePath, std::string destinationPath, unsigned long pixelPerUnit, ICloudReader *reader, ICloudSorter *sorter, ICloudRasterizer *rasterizer, IHeightMapFiller *filler, IHeightMapWriter *writer, GLHandler *glHandler)
                    : Pipeline(std::move(sourceFilePath), std::move(destinationPath), pixelPerUnit) {
    attachElements(reader, sorter, rasterizer, filler, writer);
}

void Pipeline::execute() {
    auto readerReturn = reader->apply(sourceFilePath);
    auto sorterReturn = sorter->apply(&readerReturn, pixelPerUnit);
    auto rasterizerReturn = rasterizer->apply(&sorterReturn);
    auto fillerReturn = filler->apply(&rasterizerReturn);
    writer->apply(&fillerReturn, destinationPath + "_filled");
    writer->apply(&rasterizerReturn, destinationPath);
}

void Pipeline::attachElements(ICloudReader *reader, ICloudSorter *sorter, ICloudRasterizer *rasterizer, IHeightMapFiller *filler, IHeightMapWriter *writer) {
    this->reader = reader;
    this->sorter = sorter;
    this->rasterizer = rasterizer;
    this->filler = filler;
    this->writer = writer;
}

bool Pipeline::isOperable() {
    return !(reader->isDefaultComponent()
            || sorter->isDefaultComponent()
            || rasterizer->isDefaultComponent()
            || filler->isDefaultComponent()
            || writer->isDefaultComponent());
}

ICloudReader *Pipeline::getReader() {
    return reader;
}
void Pipeline::setReader(ICloudReader *reader) {
    Pipeline::reader = reader;
}
ICloudSorter *Pipeline::getSorter() {
    return sorter;
}
void Pipeline::setSorter(ICloudSorter *sorter) {
    Pipeline::sorter = sorter;
}
ICloudRasterizer *Pipeline::getRasterizer() {
    return rasterizer;
}
void Pipeline::setRasterizer(ICloudRasterizer *rasterizer) {
    Pipeline::rasterizer = rasterizer;
}
IHeightMapFiller *Pipeline::getFiller() {
    return filler;
}
void Pipeline::setFiller(IHeightMapFiller *filler) {
    Pipeline::filler = filler;
}
IHeightMapWriter *Pipeline::getWriter() {
    return writer;
}
void Pipeline::setWriter(IHeightMapWriter *writer) {
    Pipeline::writer = writer;
}
GLHandler *Pipeline::getGLHandler() {
    return glHandler;
}
void Pipeline::setGLHandler(GLHandler *glHandler) {
    this->glHandler = glHandler;
}