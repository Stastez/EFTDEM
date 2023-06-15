#include "Pipeline.h"

#include <utility>

Pipeline::~Pipeline() {
    glHandler->uninitializeGL();
}

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

bool adjacentStagesUseGPU(IPipelineComponent *first, IPipelineComponent *second) {
    return !(first->usesGPU() && second->usesGPU());
}

void Pipeline::execute() {
    if (!isOperable()) exit(EXIT_INVALID_FUNCTION_PARAMETERS);

    bool generateOutput = true;
    generateOutput = adjacentStagesUseGPU(reader, sorter);
    auto readerReturn = reader->apply(sourceFilePath, generateOutput);
    reader->cleanUp();
    generateOutput = adjacentStagesUseGPU(sorter, rasterizer);
    auto sorterReturn = sorter->apply(&readerReturn, pixelPerUnit, generateOutput);
    sorter->cleanUp();
    generateOutput = adjacentStagesUseGPU(rasterizer, filler);
    generateOutput = true; //Closing filter not yet using previous buffers
    auto rasterizerReturn = rasterizer->apply(&sorterReturn, generateOutput);
    rasterizer->cleanUp();
    generateOutput = adjacentStagesUseGPU(filler, writer);
    auto fillerReturn = filler->apply(&rasterizerReturn, generateOutput);
    filler->cleanUp();
    writer->apply(&fillerReturn, destinationPath + "_filled", generateOutput);
    writer->apply(&rasterizerReturn, destinationPath, generateOutput);
    writer->cleanUp();
}

void Pipeline::attachElements(ICloudReader *reader, ICloudSorter *sorter, ICloudRasterizer *rasterizer, IHeightMapFiller *filler, IHeightMapWriter *writer) {
    this->reader = reader;
    this->sorter = sorter;
    this->rasterizer = rasterizer;
    this->filler = filler;
    this->writer = writer;
}

bool Pipeline::isOperable() {
    return !(reader == nullptr
            || sorter == nullptr
            || rasterizer == nullptr
            || filler == nullptr
            || writer == nullptr);
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