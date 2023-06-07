
#include "FileIO.h"
#include <gdal_priv.h>
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
rawPointCloud FileIO::readCSV(const std::string& fileName) {
    std::fstream pointFile (fileName, std::ios::in);
    if (!pointFile.is_open()) {
        std::cout << "Specified file could not be opened." << std::endl;
        exit(3);
    }

    std::vector<point> groundPoints, environmentPoints;
    std::cout << "Reading point cloud..." << std::endl;

    point min = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),std::numeric_limits<double>::max()};
    point max = {std::numeric_limits<double>::min(), std::numeric_limits<double>::min(),std::numeric_limits<double>::min()};

    std::string line;
    std::getline(pointFile, line);
    while(std::getline(pointFile, line)){
        std::stringstream str(line);

        std::string words[5];
        for (auto & word : words) std::getline(str, word, ',');

        point p = {.x=stod(words[0]), .y=stod(words[1]), .z=stod(words[2]), .intensity=stoi(words[4])};

        if (words[3] == "1") {
            min.x = std::min(min.x, p.x);
            min.y = std::min(min.y, p.y);
            min.z = std::min(min.z, p.z);
            max.x = std::max(max.x, p.x);
            max.y = std::max(max.y, p.y);
            max.z = std::max(max.z, p.z);

            groundPoints.push_back(p);
        } else environmentPoints.push_back(p);
    }

    pointFile.close();

    if (groundPoints.empty()) {
        min = {0,0,0}; max = {0,0,0};
    }

    return {groundPoints, environmentPoints, min, max};
}

/**
 * Exports the provided height map into GeoTiff format. The resolution of the height map must be within int limits.
 *
 * @param map
 * @param resolutionX
 * @param resolutionY
 */
void FileIO::writeTIFF(const heightMap *map, const std::string& destinationDEM, const bool writeLowDepth) {
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

    auto denormalizedHeights = new double[map->resolutionX * map->resolutionY];
    for (int i = 0; i < map->resolutionX * map->resolutionY; i++)
        denormalizedHeights[i] = denormalizeValue(map->heights[i], map->min.z, map->max.z);

    char** gdalDriverOptions = nullptr;
    gdalDriverOptions = CSLAddNameValue(gdalDriverOptions, "COMPRESS", "LZW");
    gdalDriverOptions = CSLAddNameValue(gdalDriverOptions, "NUM_THREADS", "16");

    auto dataset = driver->Create((destinationDEM + ".tiff").c_str(), resolutionX, resolutionY, 1, GDT_Float64,gdalDriverOptions);
    auto rasterBand = dataset->GetRasterBand(1);
    rasterBand->RasterIO(GF_Write, 0, 0, resolutionX, resolutionY, denormalizedHeights, resolutionX, resolutionY, GDT_Float64, 0, 0, nullptr);
    GDALClose(dataset);

    if (writeLowDepth) {
        std::cout << "Writing second GeoTIFF with reduced depth..." << std::endl;

        auto heightsLowDepth = new int[resolutionX * resolutionY];
        for (auto i = 0; i < resolutionX * resolutionY; i++) {
            heightsLowDepth[i] = (int) (map->heights[i] * (double) 255);
        }

        dataset = driver->Create((destinationDEM + "_lowDepth.tiff").c_str(), resolutionX, resolutionY, 1, GDT_Byte, gdalDriverOptions);
        rasterBand = dataset->GetRasterBand(1);
        rasterBand->RasterIO(GF_Write, 0, 0, resolutionX, resolutionY, heightsLowDepth, resolutionX, resolutionY, GDT_Byte, 4, 4 * resolutionX, nullptr);
        GDALClose(dataset);
    }
}
