#include "GLHandler.h"
#include "ClosingFilter.h"
#include "Pipeline.h"
#include "CloudReader.h"
#include "GTiffWriter.h"
#include "SorterCPU.h"
#include "RasterizerCpu.h"
#include "RasterizerGpu.h"
#include <iostream>

/*
 * Exit codes:
 * 0 : regular execution
 * 1 : wrong command line parameters
 * 2 : invalid parameters given to function
 * 3 : IO error
 * 4 : OpenGL error
 */

/*void readCloudAndMakeHeightMap(int argc, char** argv, GLHandler* glHandler){
    if (argc < 6) {
        std::cout << "Usage: EFTDEM <path to point cloud> <desired path to resulting DEM> <pixel per unit> <filling kernel radius in pixels> <use GPU acceleration ? 0 - no | 1 - OpenGL>" << std::endl;
        exit(1);
    }

    std::string filename = argv[1];
    std::string destinationDEM = argv[2];
    unsigned long pixelPerUnit = strtoul(argv[3], nullptr, 10);
    auto rawCloud = FileIO::readCSV(filename);

    auto grid = Rasterizer::rasterizeToPointGrid(&rawCloud, pixelPerUnit);

    auto map = Rasterizer::rasterizeToHeightMap(&grid, (bool) strtol(argv[5], nullptr, 10), glHandler);
    map = ClosingFilter::apply(&map, glHandler, strtoul(argv[4], nullptr, 10));

    FileIO::writeTIFF(&map, destinationDEM, true);
}*/

int main(int argc, char** argv) {
    if (argc < 6) {
        std::cout << "Usage: EFTDEM <path to point cloud> <desired path to resulting DEM> <pixel per unit> <filling kernel radius in pixels> <use GPU acceleration ? 0 - no | 1 - OpenGL>" << std::endl;
        exit(1);
    }

    auto pipeline = new Pipeline(argv[1], argv[2],strtoul(argv[3], nullptr, 10));

    pipeline->attachElements(
            new CloudReader(),
            new SorterCPU(),
            ((bool) strtol(argv[5], nullptr, 10)) ? (ICloudRasterizer *) new RasterizerGPU(pipeline->getGLHandler()) : (ICloudRasterizer *) new RasterizerCPU(),
            new ClosingFilter(pipeline->getGLHandler(), strtoul(argv[4], nullptr, 10), 2048),
            new GTiffWriter(true)
            );

    pipeline->execute();

    return 0;
}