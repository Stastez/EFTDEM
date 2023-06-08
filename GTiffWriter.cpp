#include <iostream>
#include "GTiffWriter.h"
#include <gdal_priv.h>

GTiffWriter::GTiffWriter(bool writeLowDepth) {
    GTiffWriter::writeLowDepth = writeLowDepth;
    GTiffWriter::stageUsesGPU = false;
}

void GTiffWriter::cleanUp() {

}

/**
 * Exports the provided height map into GeoTiff format. The resolution of the height map must be within int limits.
 *
 * @param map
 * @param resolutionX
 * @param resolutionY
 */
void GTiffWriter::apply(const heightMap *map, const std::string &destinationDEM, bool generateOutput) {
    std::cout << "Writing GeoTIFF..." << std::endl;

    if (!generateOutput) return;

    if (map->resolutionX > std::numeric_limits<int>::max() || map->resolutionY > std::numeric_limits<int>::max()) {
        std::cout << "Resolution too great for GeoTIFF!" << std::endl;
        exit(2);
    }

    int resolutionX = (int) map->resolutionX, resolutionY = (int) map->resolutionY;

    GDALRegister_GTiff();

    auto driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (driver == nullptr) { std::cout << "Could not get driver!" << std::endl; exit(3); }
    if (!CSLFetchBoolean(GDALGetMetadata(driver, nullptr), GDAL_DCAP_CREATE, FALSE)) { std::cout << "Driver does not support creation!" << std::endl; exit(3); }

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