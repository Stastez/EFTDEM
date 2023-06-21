#include "ClosingFilter.h"
#include "DataStructures.h"
#include "GLHandler.h"

ClosingFilter::ClosingFilter(GLHandler *glHandler, std::vector<unsigned int> kernelRadii, unsigned int batchSize = 0) {
    ClosingFilter::glHandler = glHandler;
    ClosingFilter::batchSize = batchSize;
    ClosingFilter::stageUsesGPU = true;

    ClosingFilter::kernelRadii = kernelRadii;

    shaderPaths = std::vector<std::string>();
    shaderPaths.emplace_back("discretization.glsl");
    shaderPaths.emplace_back("horizontalAmount.glsl");
    shaderPaths.emplace_back("amount.glsl");
    shaderPaths.emplace_back("horizontalSum.glsl");
    shaderPaths.emplace_back("sum.glsl");
    shaderPaths.emplace_back("average.glsl");

    shaderPaths.emplace_back("discretization.glsl");
    shaderPaths.emplace_back("horizontalAmount.glsl");
    shaderPaths.emplace_back("amount.glsl");
    shaderPaths.emplace_back("dilation.glsl");
    shaderPaths.emplace_back("horizontalAmount.glsl");
    shaderPaths.emplace_back("amount.glsl");
    shaderPaths.emplace_back("erosion.glsl");

    shaderPaths.emplace_back("closing.glsl");


    bufferSpecifications b;
    interimBufferSpecifications = std::vector<bufferSpecifications>();
    interimBufferSpecifications.emplace_back(bufferSpecifications{.buffer = GLHandler::EFTDEM_CLOSING_MASK_BUFFER, .elementSize = (long long) sizeof(double)});
    interimBufferSpecifications.emplace_back(bufferSpecifications{.buffer = GLHandler::EFTDEM_HORIZONTAL_AMOUNT_BUFFER, .elementSize = (long long) sizeof(unsigned int)});
    interimBufferSpecifications.emplace_back(bufferSpecifications{.buffer = GLHandler::EFTDEM_AMOUNT_BUFFER, .elementSize = (long long) sizeof(unsigned int)});
    interimBufferSpecifications.emplace_back(bufferSpecifications{.buffer = GLHandler::EFTDEM_AVERAGE_BUFFER, .elementSize = (long long) sizeof(double)});
    interimBufferSpecifications.emplace_back(bufferSpecifications{.buffer = GLHandler::EFTDEM_HORIZONTAL_SUM_BUFFER, .elementSize = (long long) sizeof(double)});
    interimBufferSpecifications.emplace_back(bufferSpecifications{.buffer = GLHandler::EFTDEM_SUM_BUFFER, .elementSize = (long long) sizeof(double)});
}

heightMap ClosingFilter::apply(heightMap *map, bool generateOutput) {
    using namespace gl;

    for (int i=0; i<kernelRadii.size()-1; i++){
        applySingleFilter(map, false, kernelRadii[i]);
    }

    return applySingleFilter(map, generateOutput, kernelRadii.back());
}