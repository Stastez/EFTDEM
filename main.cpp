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

void readCloudAndMakeHeightMap(int argc, char** argv){
    if (argc <= 3) {
        std::cout << "Usage: EFTDEM <path to point cloud> <desired path to resulting DEM> <pixel per unit>" << std::endl;
        exit(1);
    }

    std::string filename = argv[1];
    std::string destinationDEM = argv[2];
    unsigned long pixelPerUnit = strtoul(argv[3], nullptr, 10);
    auto rawCloud = fileIO::readCSV(filename);

    auto grid = rasterizer::rasterizeToPointGrid(&rawCloud, pixelPerUnit);

    auto map = rasterizer::rasterizeToHeightMap(&grid);
    fileIO::writeTIFF(&map, destinationDEM, true);
}

int main(int argc, char** argv) {
    auto gl = new glHandler();
    gl->initializeGL();
    gl->getShader("../../shaders/test.glsl");

    return 0;
}