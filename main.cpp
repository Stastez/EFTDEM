#include "GLHandler.h"
#include "ClosingFilter.h"
#include "Pipeline.h"
#include "CloudReader.h"
#include "GTiffWriter.h"
#include "SorterCPU.h"
#include "RasterizerCpu.h"
#include "RasterizerGpu.h"
#include "SorterGpu.h"
#include "ConfigProvider.h"
#include <iostream>

/*
 * Exit codes:
 * 0 : regular execution
 * 1 : wrong command line parameters
 * 2 : invalid parameters given to function
 * 3 : IO error
 * 4 : OpenGL error
 * 5 : dependency error
 */

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