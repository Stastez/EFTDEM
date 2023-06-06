#include "fileIO.h"
#include "rasterizer.h"
#include "glHandler.h"
#include <iostream>

/*
 * Exit codes:
 * 0 : regular execution
 * 1 : wrong command line parameters
 * 2 : invalid parameters given to function
 * 3 : IO error
 * 4 : OpenGL error
 */

void readCloudAndMakeHeightMap(int argc, char** argv, glHandler* glHandler){
    if (argc < 5) {
        std::cout << "Usage: EFTDEM <path to point cloud> <desired path to resulting DEM> <pixel per unit> <use GPU acceleration ? 0 - no | 1 - OpenGL>" << std::endl;
        exit(1);
    }

    std::string filename = argv[1];
    std::string destinationDEM = argv[2];
    unsigned long pixelPerUnit = strtoul(argv[3], nullptr, 10);
    auto rawCloud = fileIO::readCSV(filename);

    auto grid = rasterizer::rasterizeToPointGrid(&rawCloud, pixelPerUnit);

    auto map = rasterizer::rasterizeToHeightMap(&grid, (bool) strtol(argv[4], nullptr, 10), glHandler);
    fileIO::writeTIFF(&map, destinationDEM, true);
}

int main(int argc, char** argv) {
    auto gl = new glHandler();

    readCloudAndMakeHeightMap(argc, argv, gl);

    gl->uninitializeGL();

    return 0;
}