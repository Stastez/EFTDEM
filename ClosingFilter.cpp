#include "ClosingFilter.h"
#include "DataStructures.h"
#include "GLHandler.h"

ClosingFilter::ClosingFilter(GLHandler *glHandler, std::vector<unsigned int> kernelRadii, unsigned int batchSize = 0) {
    ClosingFilter::glHandler = glHandler;
    ClosingFilter::batchSize = batchSize;
    ClosingFilter::stageUsesGPU = true;

    ClosingFilter::kernelRadii = kernelRadii;

    shaderPaths = std::vector<std::string>();
    shaderPaths.emplace_back("../../shaders/horizontalSum.glsl");
    shaderPaths.emplace_back("../../shaders/dilation.glsl");
    shaderPaths.emplace_back("../../shaders/horizontalAmount.glsl");
    shaderPaths.emplace_back("../../shaders/erosion.glsl");

    bufferSpecifications b;
    interimBufferSpecifications = std::vector<bufferSpecifications>();
    b = {.buffer = GLHandler::EFTDEM_DILATION_HORIZONTAL_SUM_BUFFER, .elementSize = (long long) sizeof(double)};
    interimBufferSpecifications.emplace_back(b);
    b = {.buffer = GLHandler::EFTDEM_DILATION_HORIZONTAL_AMOUNT_BUFFER, .elementSize = (long long) sizeof(unsigned int)};
    interimBufferSpecifications.emplace_back(b);
    b = {.buffer = GLHandler::EFTDEM_DILATION_RESULT_BUFFER, .elementSize = (long long) sizeof(double)};
    interimBufferSpecifications.emplace_back(b);
    b = {.buffer = GLHandler::EFTDEM_EROSION_HORIZONTAL_AMOUNT_BUFFER, .elementSize = (long long) sizeof(unsigned int)};
    interimBufferSpecifications.emplace_back(b);
}

heightMap ClosingFilter::apply(heightMap *map, bool generateOutput) {
    using namespace gl;

    for (int i=0; i<kernelRadii.size()-1; i++){
        applySingleFilter(map, false, kernelRadii[i]);
    }

    return applySingleFilter(map, generateOutput, kernelRadii.back());
}