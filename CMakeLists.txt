cmake_minimum_required(VERSION 3.25)
project(EFTDEM)
set(CMAKE_CXX_STANDARD 20)

add_executable(EFTDEM fileIO.cpp fileIO.h main.cpp dataStructures.cpp rasterizer.cpp rasterizer.h)

find_package(GDAL CONFIG REQUIRED)
target_link_libraries(EFTDEM PRIVATE GDAL::GDAL)