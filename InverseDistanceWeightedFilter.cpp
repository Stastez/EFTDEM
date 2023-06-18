#include "InverseDistanceWeightedFilter.h"
#include "DataStructures.h"
#include "GLHandler.h"

InverseDistanceWeightedFilter::InverseDistanceWeightedFilter(GLHandler *glHandler, unsigned int kernelRadius, unsigned int batchSize) {
    InverseDistanceWeightedFilter::glHandler = glHandler;
    InverseDistanceWeightedFilter::batchSize = batchSize;
    InverseDistanceWeightedFilter::stageUsesGPU = true;

    InverseDistanceWeightedFilter::kernelRadius = kernelRadius;

    shaderPaths = std::vector<std::string>();
    shaderPaths.emplace_back("../../shaders/horizontalInverseDistanceSum.glsl");
    shaderPaths.emplace_back("../../shaders/inverseDistanceAverage.glsl");
    shaderPaths.emplace_back("../../shaders/horizontalAmount.glsl");
    shaderPaths.emplace_back("../../shaders/erosion.glsl");

    bufferSpecifications b;
    interimBufferSpecifications = std::vector<bufferSpecifications>();
    b = {.buffer = GLHandler::EFTDEM_SUM_BUFFER, .elementSize = (long long) sizeof(double)};
    interimBufferSpecifications.emplace_back(b);
    b = {.buffer = GLHandler::EFTDEM_TOTAL_WEIGHT_BUFFER, .elementSize = (long long) sizeof(double)};
    interimBufferSpecifications.emplace_back(b);
    b = {.buffer = GLHandler::EFTDEM_INTERIM_RESULT_BUFFER, .elementSize = (long long) sizeof(double)};
    interimBufferSpecifications.emplace_back(b);
    b = {.buffer = GLHandler::EFTDEM_AMOUNT_BUFFER, .elementSize = (long long) sizeof(unsigned int)};
    interimBufferSpecifications.emplace_back(b);
}

heightMap InverseDistanceWeightedFilter::apply(heightMap *map, bool generateOutput) {
    return applySingleFilter(map, generateOutput, kernelRadius);
}