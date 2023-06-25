#include "Pipeline.h"

#include <utility>

Pipeline::~Pipeline() {
    glHandler->uninitializeGL();
}

Pipeline::Pipeline(std::string shaderDirectory) {
    this->glHandler = new GLHandler(std::move(shaderDirectory));
    this->glHandler->initializeGL(false);
}

bool adjacentStagesUseGPU(IPipelineComponent *first, IPipelineComponent *second) {
    return !(first->usesGPU() && second->usesGPU());
}

void Pipeline::execute() {
    if (!isOperable()) exit(EXIT_INVALID_FUNCTION_PARAMETERS);

    bool generateOutput = adjacentStagesUseGPU(reader, sorter);
    auto readerReturn = reader->apply(generateOutput);
    reader->cleanUp();
    generateOutput = adjacentStagesUseGPU(sorter, rasterizer);
    auto sorterReturn = sorter->apply(&readerReturn, generateOutput);
    sorter->cleanUp();
    readerReturn = {};
    generateOutput = adjacentStagesUseGPU(rasterizer, filler);
    auto rasterizerReturn = rasterizer->apply(&sorterReturn, generateOutput);
    rasterizer->cleanUp();
    sorterReturn = {};
    generateOutput = adjacentStagesUseGPU(filler, writer);
    auto fillerReturn = filler->apply(&rasterizerReturn, generateOutput);
    filler->cleanUp();
    rasterizerReturn = {};
    writer->apply(&fillerReturn, generateOutput);
    writer->cleanUp();
    fillerReturn = {};
}

void Pipeline::attachElements(ICloudReader *readerParameter, ICloudSorter *sorterParameter, ICloudRasterizer *rasterizerParameter, IHeightMapFiller *fillerParameter, IHeightMapWriter *writerParameter) {
    this->reader = readerParameter;
    this->sorter = sorterParameter;
    this->rasterizer = rasterizerParameter;
    this->filler = fillerParameter;
    this->writer = writerParameter;
}

bool Pipeline::isOperable() {
    return !(reader == nullptr
            || sorter == nullptr
            || rasterizer == nullptr
            || filler == nullptr
            || writer == nullptr);
}