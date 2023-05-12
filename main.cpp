#include "csvReader.h"
#include "rasterizer.h"
#include <iostream>
#include <fstream>

/*
 * Exit codes:
 * 0 : regular execution
 * 1 : wrong command line parameters
 * 2 : invalid parameters given to function
 * 3 : IO error
 */

int main(int argc, char** argv) {
    if (argc <= 3) {
        std::cout << "Usage: eftdem <path to point cloud> <resolutionX> <resolutionZ>" << std::endl;
        exit(1);
    }

    std::string filename = argv[1];
    unsigned long resolutionX = strtoul(argv[2], nullptr, 10), resolutionZ = strtoul(argv[3], nullptr, 10);
    auto reader = new csvReader();
    auto rawCloud = reader->readCSV(filename);

    auto rasterizer = new class rasterizer();
    auto grid = rasterizer->rasterizeToPointGrid(rawCloud, resolutionX, resolutionZ);

    std::fstream f ("../test.txt", std::ios::out);
    std::string intro = "X,Y,Z,SemClassID,Intensity\n";
    f.write(intro.c_str(), intro.size());
    for (auto i = 0; i < resolutionZ * resolutionX; i++) {
        for (auto it = grid.points[i].begin(); it != grid.points[i].end(); it++) {
            std::string s = std::to_string(it->x) + "," + std::to_string(it->y) + "," + std::to_string(it->z) + ",1," + std::to_string(it->intensity) + "\n";
            f.write(s.c_str(), s.size());
        }
    }
    f.close();

    return 0;
}