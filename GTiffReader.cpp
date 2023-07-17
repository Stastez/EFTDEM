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

    GDALAllRegister();//!!!!!!!!!! GDALRegister_GTiff();
    GDALDataset * dataset = (GDALDataset *) GDALOpen(sourceDEM.c_str(), GA_ReadOnly);

    resolutionX = dataset->GetRasterXSize();
    resolutionY = dataset->GetRasterYSize();

    auto rasterBand = dataset->GetRasterBand(1);

    auto heights = std::vector<double>(resolutionX * resolutionY);
    (void)! rasterBand->RasterIO(GF_Read, 0, 0, resolutionX, resolutionY, heights.data(), resolutionX, resolutionY, GDT_Float64, 0, 0, nullptr);

    double minHeight = std::numeric_limits<double>::max(), maxHeight = 0;
    for (auto i = 0ul; i < resolutionY; i++){
        minHeight = std::min(minHeight, heights.at(i));
        maxHeight = std::max(maxHeight, heights.at(i));
    }

    for (auto y = 0ul; y < (resolutionY / 2); y++){
        for (auto x = 0ul; x < resolutionX; x++) {
            auto coord1D = resolutionX * y + x;
            auto flippedCoord1D = resolutionX * (resolutionY-1 - y) + x;

            auto temp = heights.at(flippedCoord1D);
            heights.at(flippedCoord1D) = heights.at(coord1D);
            heights.at(coord1D) = temp;
        }
    }

    return new denormalizedHeightMap{
            .heights = heights,
            .resolutionX = resolutionX,
            .resolutionY = resolutionY,
            .dataSize = (long) (resolutionX * resolutionY * sizeof(float)),
            .min = doublePoint{std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), minHeight},
            .max = doublePoint{std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), maxHeight}
    };
}