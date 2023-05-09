cmake_minimum_required(VERSION 3.25)
project(EFTDEM)

set(CMAKE_CXX_STANDARD 20)

add_executable(EFTDEM csvReader.cpp csvReader.h main.cpp)