#include "FileIO.h"
#include "Rasterizer.h"
#include "GLHandler.h"
#include "Filler.h"
#include <iostream>

/*
 * Exit codes:
 * 0 : regular execution
 * 1 : wrong command line parameters
 * 2 : invalid parameters given to function
 * 3 : IO error
 * 4 : OpenGL error
 */

void readCloudAndMakeHeightMap(int argc, char** argv, GLHandler* glHandler){
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
    map = Filler::applyClosingFilter(&map, glHandler, strtoul(argv[4], nullptr, 10));

    FileIO::writeTIFF(&map, destinationDEM, true);
}

int main(int argc, char** argv) {
    auto gl = new GLHandler();

    readCloudAndMakeHeightMap(argc, argv, gl);

    gl->uninitializeGL();

    return 0;
}