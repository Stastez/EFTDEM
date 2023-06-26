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

    bool generateAllOutputs = true;

    bool generateOutput = adjacentStagesUseGPU(reader, sorter) || generateAllOutputs;
    auto readerReturn = reader->apply(generateOutput);
    delete reader;
    generateOutput = adjacentStagesUseGPU(sorter, rasterizer) || generateAllOutputs;
    auto sorterReturn = sorter->apply(&readerReturn, generateOutput);
    delete sorter;
    readerReturn = {};
    generateOutput = adjacentStagesUseGPU(rasterizer, filler) || generateAllOutputs;
    auto rasterizerReturn = rasterizer->apply(&sorterReturn, generateOutput);
    delete rasterizer;
    sorterReturn = {};
    generateOutput = adjacentStagesUseGPU(filler, writer) || generateAllOutputs;
    auto fillerReturn = filler->apply(&rasterizerReturn, generateOutput);
    delete filler;
    rasterizerReturn = {};
    writer->apply(&fillerReturn, generateOutput);
    delete writer;
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