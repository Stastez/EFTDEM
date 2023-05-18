
#include "fileIO.h"
#include "gdal_priv.h"
#include <fstream>
#include <iostream>
#include <sstream>

using std::vector, std::pair;

/**
 * Reads the content of the fileName-provided file into a raw, non-grid point cloud struct. The file must be comma-
 * separated and conform to the format [x],[y],[z],[ground point? -> 1; environment point? -> 0],[reflection intensity]
 * @param fileName The path to the csv containing the point cloud
 * @return A new rawPointCloud struct
 */
rawPointCloud fileIO::readCSV(const std::string& fileName) {
    std::fstream pointFile (fileName, std::ios::in);
    if (!pointFile.is_open()) {
        std::cout << "Specified file could not be opened." << std::endl;
        exit(3);
    }

    std::cout << "Reading point cloud..." << std::endl;

    double minX = std::numeric_limits<double>::max(), minY = std::numeric_limits<double>::max(), maxX = std::numeric_limits<double>::min(), maxY = std::numeric_limits<double>::min();

    std::string line;
    std::getline(pointFile, line);
    while(std::getline(pointFile, line)){
        std::stringstream str(line);

        std::string words[5];
        for (auto & word : words) std::getline(str, word, ',');

        point p = {.x=stod(words[0]), .y=stod(words[1]), .z=stod(words[2]), .intensity=stoi(words[4])};

        minX = std::min(minX, p.x); minY = std::min(minY, p.y); maxX = std::max(maxX, p.x); maxY = std::max(maxY, p.y);

        if (words[3] == "1") groundPoints.push_back(p);
        else environmentPoints.push_back(p);
    }

    pointFile.close();

    if (groundPoints.empty()) {
        minX = 0; maxX = 0; minY = 0; maxY = 0;
    }

    return {&groundPoints, &environmentPoints, minX, maxX, minY, maxY};
}

/**
 * Exports the provided height map into GeoTiff format. The resolution of the height map must be within int limits.
 *
 * @param map
 * @param resolutionX
 * @param resolutionY
 */
void fileIO::writeTIFF(const heightMap *map, const bool writeLowDepth) {
    if (map->resolutionX > std::numeric_limits<int>::max() || map->resolutionY > std::numeric_limits<int>::max()) {
        std::cout << "Resolution too great for GeoTIFF!" << std::endl;
        exit(2);
    }

    int resolutionX = (int) map->resolutionX, resolutionY = (int) map->resolutionY;

    GDALRegister_GTiff();

    auto driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (driver == nullptr) { std::cout << "Could not get driver!" << std::endl; exit(3); }
    if (!CSLFetchBoolean(GDALGetMetadata(driver, nullptr), GDAL_DCAP_CREATE, FALSE)) { std::cout << "Driver does not support creation!" << std::endl; exit(3); }

    std::cout << "Writing GeoTIFF..." << std::endl;

    auto dataset = driver->Create("../test.tiff", resolutionX, resolutionY, 1, GDT_Float64, nullptr);
    auto rasterBand = dataset->GetRasterBand(1);
    rasterBand->RasterIO(GF_Write, 0, 0, resolutionX, resolutionY, map->heights, resolutionX, resolutionY, GDT_Float64, 0, 0, nullptr);
    GDALClose(dataset);

    if (writeLowDepth) {
        std::cout << "Writing second GeoTIFF with reduced depth..." << std::endl;

        double max = 0, min = std::numeric_limits<double>::max();
        for (auto i = 0; i < resolutionX * resolutionY; i++) {
            max = std::max(max, map->heights[i]);
            min = std::min(min, map->heights[i]);
        }
        auto heightsLowDepth = new int[resolutionX * resolutionY];
        for (auto i = 0; i < resolutionX * resolutionY; i++) {
            heightsLowDepth[i] = (int) (((map->heights[i] - min) / (max - min)) * (double) 255);
        }

        dataset = driver->Create("../test_lowDepth.tiff", resolutionX, resolutionY, 1, GDT_Byte, nullptr);
        rasterBand = dataset->GetRasterBand(1);
        rasterBand->RasterIO(GF_Write, 0, 0, resolutionX, resolutionY, heightsLowDepth, resolutionX, resolutionY,
                             GDT_Byte, 4, 4 * resolutionX, nullptr);
        GDALClose(dataset);
    }
}
