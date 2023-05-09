#include "csvReader.h"
#include <iostream>

int main(int argc, char** argv) {
    csvReader *reader = new csvReader();
    auto v = reader->readCSV("../" + (std::string) argv[1]);

    std::cout << v.first->at(0).intensity;

    return 0;
}