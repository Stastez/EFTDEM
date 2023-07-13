#include "GTiffReader.h"
#include "Pipeline.h"
#include "DataStructures.h"
#include <iostream>
#include <gdal_priv.h>
#include <cmath>

GTiffReader::GTiffReader(const std::string& sourceDEM) {
    GTiffReader::sourceDEM = sourceDEM;
    GTiffReader::stageUsesGPU = false;
}

GTiffReader::~GTiffReader() noexcept = default;

void GTiffReader::setSourceDEM(const std::string &soureDEM) {
    GTiffReader::sourceDEM = soureDEM;
}

/**
 * Exports the provided height map into GeoTiff format. The resolution of the height map must be within int limits.
 *
 * @param map
 * @param resolutionX
 * @param resolutionY
 */
denormalizedHeightMap * GTiffReader::apply(bool generateOutput) {

    std::cout << "Reading GeoTIFF..." << std::endl;

    if (!generateOutput) return nullptr;

    unsigned long resolutionX, resolutionY;

    GDALRegister_GTiff();

    auto driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (driver == nullptr) { std::cout << "Could not get driver!" << std::endl; exit(Pipeline::EXIT_IO_ERROR); }
    //if (!CSLFetchBoolean(GDALGetMetadata(driver, nullptr), GDAL_DCAP_CREATE, FALSE)) { std::cout << "Driver does not support creation!" << std::endl; exit(Pipeline::EXIT_IO_ERROR); }

    //char** gdalDriverOptions = nullptr;
    //gdalDriverOptions = CSLAddNameValue(gdalDriverOptions, "COMPRESS", "LZW");
    //gdalDriverOptions = CSLAddNameValue(gdalDriverOptions, "NUM_THREADS", "16");

    GDALAllRegister();//!!!!!!!!!! GDALRegister_GTiff();
    GDALDataset * dataset = (GDALDataset *) GDALOpen(sourceDEM.c_str(), GA_ReadOnly);

    resolutionX = dataset->GetRasterXSize();
    resolutionY = dataset->GetRasterYSize();

    auto rasterBand = dataset->GetRasterBand(1);

    auto heights = std::vector<double>(resolutionX * resolutionY);
    (void)! rasterBand->RasterIO(GF_Read, 0, 0, resolutionX, resolutionY, heights.data(), resolutionX, resolutionY, GDT_Float64, 0, 0, nullptr);

    double minHeight = std::numeric_limits<double>::max(), maxHeight = 0;
    for (auto i = 0ul; i < resolutionX * resolutionY; i++){
        minHeight = std::min(minHeight, heights.at(i));
        maxHeight = std::max(maxHeight, heights.at(i));
    }

    return new denormalizedHeightMap{
            .heights = heights,
            .resolutionX = resolutionX,
            .resolutionY = resolutionY,
            .dataSize = (long) (resolutionX * resolutionY * sizeof(float)),
            .min = doublePoint{std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), minHeight},
            .max = doublePoint{std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), maxHeight}
    };


    /*GDALDataset * dataset = GDALOpen((sourceDEM + ".tiff").c_str(),GA_ReadOnly);// driver->pfnOpen((sourceDEM + ".tiff").c_str(), resolutionX, resolutionY, 1, GDT_Float64, gdalDriverOptions);
    auto rasterBand = dataset->GetRasterBand(1);
    (void)! rasterBand->RasterIO(GF_Read, 0, 0, resolutionX, resolutionY, heights, resolutionX, resolutionY, GDT_Float64, 0, 0, nullptr);
    GDALClose(dataset);

    double * normalizedHeights;
    double minX, minY, maxX, maxY;

    for (int i = 0; i < resolutionX * resolutionY; i++){
        minX = std::min(minX, heights[i]);
        minY = std::max(minY, heights[i]);
        maxX = std::max(maxX, heights[i]);
        maxY = std::max(maxY, heights[i]);
    }

    for (int i = 0; i < resolutionX * resolutionY; i++){
    }


    return * heightMap{ .heights = normalizedHeights,
                        .resolutionX = resolutionX,
                        .resolutionY = resolutionY,
                        .dataSize = static_cast<long long>(sizeof(double) * resolutionX * resolutionY),
                        .min = point{minX, minY, },
                        .max =  };*/


}