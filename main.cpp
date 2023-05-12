#include "csvReader.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc <= 3) {
        std::cout << "Usage: eftdem <path to point cloud> <resolutionX> <resolutionZ>" << std::endl;
        exit(-1);
    }

    std::string filename = argv[1];
    unsigned long resolutionX = strtoul(argv[2], nullptr, 10), resolutionZ = strtoul(argv[3], nullptr, 10);



    return 0;
}