#include "Pipeline.h"

Pipeline::~Pipeline() {
    glHandler->uninitializeGL();
}

Pipeline::Pipeline(const std::string& shaderDirectory) : Pipeline(new GLHandler(shaderDirectory)) {}

Pipeline::Pipeline(GLHandler *glHandler) {
    Pipeline::glHandler = glHandler;
    Pipeline::glHandler->initializeGL(false);
}

bool adjacentStagesUseGPU(IPipelineComponent *first, IPipelineComponent *second) {
    return !(first->usesGPU() && second->usesGPU());
}

heightMap Pipeline::execute() {
    if (!isOperable()) exit(EXIT_INVALID_FUNCTION_PARAMETERS);

    bool generateAllOutputs = false;

    bool generateOutput = adjacentStagesUseGPU(reader, sorter) || generateAllOutputs;
    auto readerReturn = reader->apply(generateOutput);
    delete reader;
    generateOutput = adjacentStagesUseGPU(sorter, rasterizer) || generateAllOutputs;
    auto sorterReturn = sorter->apply(readerReturn, generateOutput);
    delete sorter;
    delete readerReturn;
    generateOutput = adjacentStagesUseGPU(rasterizer, filler) || generateAllOutputs;
    auto rasterizerReturn = rasterizer->apply(sorterReturn, generateOutput);
    delete rasterizer;
    delete sorterReturn;
    generateOutput = adjacentStagesUseGPU(filler, writer) || generateAllOutputs;
    auto fillerReturn = filler->apply(&rasterizerReturn, generateOutput);
    delete filler;
    //writer->apply(&rasterizerReturn, true);
    rasterizerReturn = {};
    writer->apply(&fillerReturn, generateOutput);
    delete writer;
    return fillerReturn;
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