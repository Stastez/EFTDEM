#include "ConfigProvider.h"
#include "RadarComparator.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: EFTDEM <path to config yaml>" << std::endl;
        exit(Pipeline::EXIT_INVALID_COMMAND_LINE_ARGUMENTS);
    }

    auto configProvider = new ConfigProvider(argv[1]);
    auto pipeline = configProvider->providePipeline();
    pipeline->execute();
    delete pipeline;
    delete configProvider;
    return 0;

    /*auto comparator = new RadarComparator(std::vector<std::string>(argv + 1, argv + argc));

    auto comparisons = comparator->compareMaps();
    comparator->writeComparisons(comparisons);*/
}