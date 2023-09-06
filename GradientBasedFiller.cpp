#include "GradientBasedFiller.h"

GradientBasedFiller::GradientBasedFiller(GLHandler *glHandler, unsigned int kernelRadius, unsigned int batchSize = 0) {
    GradientBasedFiller::glHandler = glHandler;
    GradientBasedFiller::kernelRadius = kernelRadius;
    GradientBasedFiller::batchSize = batchSize;
    GradientBasedFiller::stageUsesGPU = true;
}

GradientBasedFiller::~GradientBasedFiller() {
    //TODO: delete used buffers
    /*glHandler->deleteBuffer(GLHandler::EFTDEM_CLOSING_MASK_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_HORIZONTAL_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_TOTAL_WEIGHT_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_SUM_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_AVERAGE_BUFFER);*/
};

