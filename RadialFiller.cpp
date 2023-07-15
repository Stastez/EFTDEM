#include "RadialFiller.h"
#include "FillerLoop.h"
#include "RadialDilator.h"
#include "RadialEroder.h"
#include "RadialBufferSwapper.h"

RadialFiller::RadialFiller(GLHandler * glHandler, unsigned int maxHoleRadius, bool batched, unsigned int batchSize) {
    RadialFiller::glHandler = glHandler;
    RadialFiller::batched = batched;
    RadialFiller::batchSize = batchSize;
    RadialFiller::maxHoleRadius = maxHoleRadius;
    RadialFiller::stageUsesGPU = true;
}

heightMap *RadialFiller::apply(heightMap *map, bool generateOutput) {
    auto fillers = new std::vector<IHeightMapFiller *>();
    fillers->reserve(maxHoleRadius * 2 + 1);

    auto dilationProgram = glHandler->getShaderPrograms({"radialDilation.glsl"}, true).at(0);
    auto dilationResolutionLocation = gl::glGetUniformLocation(dilationProgram, "resolution");
    auto dilationCurrentInvocationLocation = gl::glGetUniformLocation(dilationProgram, "currentInvocation");
    auto dilationFlippedLocation = gl::glGetUniformLocation(dilationProgram, "flipped");

    for (auto i = 0u; i < maxHoleRadius; i++) {
        fillers->emplace_back(new RadialDilator(glHandler, (bool) (i % 2u), batchSize, batched,
                                                dilationProgram,
                                                dilationResolutionLocation,
                                                dilationCurrentInvocationLocation,
                                                dilationFlippedLocation, true));
    }

    auto erosionProgram = glHandler->getShaderPrograms({"radialErosion.glsl"}, true).at(0);
    auto erosionResolutionLocation = gl::glGetUniformLocation(erosionProgram, "resolution");
    auto erosionCurrentInvocationLocation = gl::glGetUniformLocation(erosionProgram, "currentInvocation");
    auto erosionFlippedLocation = gl::glGetUniformLocation(erosionProgram, "flipped");

    for (auto i = 0u; i < maxHoleRadius; i++) {
        fillers->emplace_back(new RadialEroder(glHandler, (bool) ((maxHoleRadius + i) % 2u), batchSize, batched,
                                               erosionProgram,
                                               erosionResolutionLocation,
                                               erosionCurrentInvocationLocation,
                                               erosionFlippedLocation, i != maxHoleRadius - 1));
    }

    if (maxHoleRadius % 2u == 1) fillers->emplace_back(new RadialBufferSwapper(glHandler));

    auto filler = new FillerLoop(*fillers);
    auto output = filler->apply(map, generateOutput);

    delete filler;
    delete fillers;

    return output;
}