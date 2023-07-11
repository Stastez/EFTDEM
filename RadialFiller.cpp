#include "RadialFiller.h"
#include "FillerLoop.h"
#include "IRadialFillerComponent.h"
#include "RadialDilator.h"
#include "RadialEroder.h"
#include "RadialBufferSwapper.h"

RadialFiller::RadialFiller(GLHandler * glHandler, unsigned int maxHoleRadius, bool batched, unsigned int batchSize) {
    RadialFiller::glHandler = glHandler;
    RadialFiller::batched = batched;
    RadialFiller::batchSize = batchSize;
    RadialFiller::maxHoleRadius = maxHoleRadius;
}

heightMap *RadialFiller::apply(heightMap *map, bool generateOutput) {
    auto fillers = new std::vector<IHeightMapFiller *>();
    fillers->reserve(maxHoleRadius * 2 + 1);

    for (auto i = 0u; i < maxHoleRadius; i++) {
        fillers->emplace_back(new RadialDilator(glHandler, (bool) (i % 2u), batchSize, batched));
    }
    for (auto i = 0u; i < maxHoleRadius; i++) {
        fillers->emplace_back(new RadialEroder(glHandler, (bool) ((maxHoleRadius + i) % 2u), batchSize, batched));
    }

    if (maxHoleRadius % 2u == 1) fillers->emplace_back(new RadialBufferSwapper(glHandler));

    auto filler = new FillerLoop(*fillers);
    auto output = filler->apply(map, generateOutput);

    delete filler;
    delete fillers;

    return output;
}