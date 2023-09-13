#include "Pipeline.h"

Pipeline::Pipeline(const std::string& shaderDirectory) : Pipeline(new GLHandler(shaderDirectory)) {}

Pipeline::Pipeline(GLHandler *glHandler) {
    Pipeline::glHandler = glHandler;
}

Pipeline::~Pipeline() {
    delete glHandler;
    delete reader;
    delete sorter;
    delete rasterizer;
    delete filler;
    delete writer;
}

bool adjacentStagesUseGPU(IPipelineComponent *first, IPipelineComponent *second) {
    return first->usesGPU() && second->usesGPU();
}

/**
 * Executes all attached IPipelineComponents sequentially and deletes interim results.
 * @return The filled heightMap
 */
heightMap * Pipeline::execute() {
    if (!isOperable()) exit(EXIT_INVALID_FUNCTION_PARAMETERS);
    glHandler->initializeGL(false);

    bool generateAllOutputs = false;

    bool generateOutput = !adjacentStagesUseGPU(reader, sorter) || generateAllOutputs;
    auto readerReturn = reader->apply(generateOutput);
    return executeAfterReader(readerReturn);
}

/**
 * Executes all attached IPipelineComponents after the ICloudReader sequentially and deletes interim results.
 * @param pointCloud The pointCloud to execute the IPipelineComponents on
 * @return The filled heightMap
 */
heightMap *Pipeline::executeAfterReader(rawPointCloud *pointCloud) {
    if (!isOperable()) exit(EXIT_INVALID_FUNCTION_PARAMETERS);
    if (!glHandler->isInitialized(false)) glHandler->initializeGL(false);

    bool generateAllOutputs = false;

    delete reader;
    reader = nullptr;
    auto generateOutput = !adjacentStagesUseGPU(sorter, rasterizer) || generateAllOutputs;
    auto sorterReturn = sorter->apply(pointCloud, generateOutput);
    delete sorter;
    sorter = nullptr;
    delete pointCloud;
    generateOutput = !adjacentStagesUseGPU(rasterizer, filler) || generateAllOutputs;
    auto rasterizerReturn = rasterizer->apply(sorterReturn, generateOutput);
    delete rasterizer;
    rasterizer = nullptr;
    delete sorterReturn;
    generateOutput = !adjacentStagesUseGPU(filler, writer) || generateAllOutputs;
    auto fillerReturn = filler->apply(rasterizerReturn, generateOutput);
    delete filler;
    filler = nullptr;
    //writer->apply(&rasterizerReturn, true);
    delete rasterizerReturn;
    writer->apply(fillerReturn, generateOutput);
    delete writer;
    writer = nullptr;
    return fillerReturn;
}

/**
 * Attach the given IPipelineComponents to the pipeline. The execution order is equal to the order of the parameters of this function.
 */
void Pipeline::attachElements(ICloudReader *readerParameter, ICloudSorter *sorterParameter, ICloudRasterizer *rasterizerParameter, IHeightMapFiller *fillerParameter, IHeightMapWriter *writerParameter) {
    this->reader = readerParameter;
    this->sorter = sorterParameter;
    this->rasterizer = rasterizerParameter;
    this->filler = fillerParameter;
    this->writer = writerParameter;
}

/**
 * Get whether all IPipelineComponent slots have an actual object attached.
 * @return Whether all IPipelineComponent slots have an actual object attached.
 */
bool Pipeline::isOperable() {
    return !(reader == nullptr
            || sorter == nullptr
            || rasterizer == nullptr
            || filler == nullptr
            || writer == nullptr);
}

GLHandler * Pipeline::getGLHandler(){ return glHandler;}
ICloudReader * Pipeline::getCloudReader() { return reader; }
ICloudSorter * Pipeline::getCloudSorter() { return sorter; }
IHeightMapWriter *Pipeline::getHeightMapWriter() {return writer;}
