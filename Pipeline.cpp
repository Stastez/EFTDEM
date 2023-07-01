#include "Pipeline.h"
#include <unistd.h>

Pipeline::~Pipeline() {
    glHandler->uninitializeGL();
}

Pipeline::Pipeline(const std::string& shaderDirectory) : Pipeline(new GLHandler(shaderDirectory)) {}

Pipeline::Pipeline(GLHandler *glHandler) {
    Pipeline::glHandler = glHandler;
}

bool adjacentStagesUseGPU(IPipelineComponent *first, IPipelineComponent *second) {
    return !(first->usesGPU() && second->usesGPU());
}

heightMap * Pipeline::execute() {
    if (!isOperable()) exit(EXIT_INVALID_FUNCTION_PARAMETERS);
    glHandler->initializeGL(false);

    bool generateAllOutputs = false;

    bool generateOutput = adjacentStagesUseGPU(reader, sorter) || generateAllOutputs;
    auto readerReturn = reader->apply(generateOutput);
    return executeAfterReader(readerReturn);
}

heightMap *Pipeline::executeAfterReader(rawPointCloud *pointCloud) {
    if (!isOperable()) exit(EXIT_INVALID_FUNCTION_PARAMETERS);
    if (!glHandler->isInitialized(false)) glHandler->initializeGL(false);

    bool generateAllOutputs = true;

    delete reader;
    auto generateOutput = adjacentStagesUseGPU(sorter, rasterizer) || generateAllOutputs;
    //usleep(500000);
    auto sorterReturn = sorter->apply(pointCloud, generateOutput);
    delete sorter;
    delete pointCloud;
    generateOutput = adjacentStagesUseGPU(rasterizer, filler) || generateAllOutputs;
    auto rasterizerReturn = rasterizer->apply(sorterReturn, generateOutput);
    delete rasterizer;
    delete sorterReturn;
    generateOutput = adjacentStagesUseGPU(filler, writer) || generateAllOutputs;
    auto fillerReturn = filler->apply(rasterizerReturn, generateOutput);
    delete filler;
    //writer->apply(&rasterizerReturn, true);
    delete rasterizerReturn;
    writer->apply(fillerReturn, generateOutput);
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
