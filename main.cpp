#include "ConfigProvider.h"
#include "RadarComparator.h"
#include "GroundTruthComparator.h"
#include <iostream>

int main(int argc, char** argv) {

    if (argc < 2) {
        std::cout << "Usage: EFTDEM <path to config yaml> or EFTDEM <path to first layer from top config yaml> <path to second layer from top config yaml> ..." << std::endl;
        exit(Pipeline::EXIT_INVALID_COMMAND_LINE_ARGUMENTS);
    } else if (argc == 2) {
        auto configProvider = new ConfigProvider(argv[1]);
        auto pipeline = configProvider->providePipeline();
        delete pipeline->execute();
        delete pipeline;
        delete configProvider;
    } else {
        bool groundTruthCompare = false;
        for (int i = 1; i < argc; i++){
            std::string s = argv[i];
            std::string suffix = s.substr(s.length() - 5, s.length() - 1);
            if (suffix == ".tiff") { groundTruthCompare = true; }
        }

        IComparator * comparator;
        if (!groundTruthCompare)
            comparator = (IComparator *) new RadarComparator(std::vector<std::string>(argv + 1, argv + argc));
        else
            comparator = (IComparator *) new GroundTruthComparator(std::vector<std::string>(argv + 1, argv + argc));

        auto comparisons = comparator->compareMaps();
        comparator->writeComparisons(comparisons);

        delete comparator;
    }

    return 0;
}