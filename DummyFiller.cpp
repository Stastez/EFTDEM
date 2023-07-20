#include "DummyFiller.h"

DummyFiller::DummyFiller() {
    stageUsesGPU = false;
}

heightMap *DummyFiller::apply(heightMap *map, bool generateOutput) {
    auto newMap = emptyHeightMapFromHeightMap(map);
    if (generateOutput) {
        newMap->heights.clear();
        newMap->heights.insert(newMap->heights.begin(), map->heights.begin(), map->heights.end());
    }
    return newMap;
}