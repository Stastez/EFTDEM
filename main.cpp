#include "ConfigProvider.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: EFTDEM <path to config yaml>" << std::endl;
        exit(Pipeline::EXIT_INVALID_COMMAND_LINE_ARGUMENTS);
    }

    auto configProvider = new ConfigProvider(argv[1]);
    auto pipeline = configProvider->providePipeline();
    pipeline->execute();

    return 0;
}