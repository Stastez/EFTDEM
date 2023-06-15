#include "GLHandler.h"
#include "ClosingFilter.h"
#include "Pipeline.h"
#include "CloudReader.h"
#include "GTiffWriter.h"
#include "SorterCPU.h"
#include "RasterizerCpu.h"
#include "RasterizerGpu.h"
#include "SorterGpu.h"
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
    if (argc < 6) {
        std::cout << "Usage: EFTDEM <path to point cloud> <desired path to resulting DEM> <pixel per unit> <filling kernel radius in pixels> <use GPU acceleration ? 0 - no | 1 - OpenGL>" << std::endl;
        exit(1);
    }

    auto pipeline = new Pipeline(argv[1], argv[2],strtoul(argv[3], nullptr, 10));

    pipeline->attachElements(
            new CloudReader(),
            new SorterGPU(pipeline->getGLHandler()),
            //new SorterCPU(),
            //((bool) strtol(argv[5], nullptr, 10)) ? (ICloudRasterizer *) new RasterizerGPU(pipeline->getGLHandler()) : (ICloudRasterizer *) new RasterizerCPU(),
            new RasterizerGPU(pipeline->getGLHandler()),
            new ClosingFilter(pipeline->getGLHandler(), strtoul(argv[4], nullptr, 10), 512),
            new GTiffWriter(true)
            );

    pipeline->execute();

    return 0;
}