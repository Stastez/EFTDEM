#include "Pipeline.h"

Pipeline::Pipeline(ICloudReader *reader, ICloudSorter *sorter, ICloudRasterizer *rasterizer, IHeightMapFiller *filler,
                   IHeightMapWriter *writer) : Pipeline(reader, sorter, rasterizer, filler, writer, nullptr) {}

Pipeline::Pipeline(ICloudReader *reader, ICloudSorter *sorter, ICloudRasterizer *rasterizer, IHeightMapFiller *filler,
                   IHeightMapWriter *writer, GLHandler *glHandler) {
    this->reader = reader;
    this->sorter = sorter;
    this->rasterizer = rasterizer;
    this->filler = filler;
    this->writer = writer;
    this->glHandler = glHandler;
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