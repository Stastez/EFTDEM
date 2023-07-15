#include "GTiffWriter.h"
#include "Pipeline.h"
#include <iostream>
#include <gdal_priv.h>

GTiffWriter::GTiffWriter(bool writeLowDepth, const std::string &destinationDEM, bool betterCompression) {
    GTiffWriter::destinationDEM = destinationDEM;
    GTiffWriter::writeLowDepth = writeLowDepth;
    GTiffWriter::stageUsesGPU = false;
    GTiffWriter::compressionAlgorithm = (betterCompression) ? "LZMA" : "LZW";
}

GTiffWriter::~GTiffWriter() noexcept = default;

void GTiffWriter::setDestinationDEM(const std::string &newDestinationDEM) {
    GTiffWriter::destinationDEM = newDestinationDEM;
}

/**
 * Exports the provided height map into GeoTiff format. The resolution of the height map must be within int limits.
 *
 * @param map
 * @param resolutionX
 * @param resolutionY
 */
void GTiffWriter::apply(const heightMap *map, bool generateOutput) {
    std::cout << "Writing GeoTIFF..." << std::endl;

    if (!generateOutput) return;

    if (map->resolutionX > (unsigned int) std::numeric_limits<int>::max() || map->resolutionY > (unsigned int) std::numeric_limits<int>::max()) {
        std::cout << "Resolution too great for GeoTIFF!" << std::endl;
        exit(Pipeline::EXIT_INVALID_FUNCTION_PARAMETERS);
    }

    int resolutionX = (int) map->resolutionX, resolutionY = (int) map->resolutionY;

    GDALRegister_GTiff();

    auto driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (driver == nullptr) { std::cout << "Could not get driver!" << std::endl; exit(Pipeline::EXIT_IO_ERROR); }
    if (!CSLFetchBoolean(GDALGetMetadata(driver, nullptr), GDAL_DCAP_CREATE, FALSE)) { std::cout << "Driver does not support creation!" << std::endl; exit(Pipeline::EXIT_IO_ERROR); }

    auto denormalizedHeights = new double[map->resolutionX * map->resolutionY];
    auto arrayIndex = 0ul;
    /*
     * GDAL expects the (0,0) pixel in the top left corner, our code assumes (0,0) in the bottom left.
     * The following code flips the image vertically to translate our coordinate system to GDAL's.
    */
    for (auto y = map->resolutionY; y > 0ul; y--) {
        for (auto x = 0ul; x < map->resolutionX; x++) {
            denormalizedHeights[arrayIndex++] = denormalizeValue(map->heights.at(calculate1DCoordinate(map, x, y - 1)), map->min.z, map->max.z);
        }
    }

    char** gdalDriverOptions = nullptr;
    gdalDriverOptions = CSLAddNameValue(gdalDriverOptions, "COMPRESS", compressionAlgorithm.c_str());
    gdalDriverOptions = CSLAddNameValue(gdalDriverOptions, "NUM_THREADS", "16");

    auto dataset = driver->Create((destinationDEM + ".tiff").c_str(), resolutionX, resolutionY, 1, GDT_Float64,gdalDriverOptions);
    if (dataset == nullptr) {
        std::cout << "WARNING :: GeoTIFF dataset could not be created at " << destinationDEM << ".tiff!" << std::endl;
        return;
    }
    auto rasterBand = dataset->GetRasterBand(1);
    (void)! rasterBand->RasterIO(GF_Write, 0, 0, resolutionX, resolutionY, denormalizedHeights, resolutionX, resolutionY, GDT_Float64, 0, 0, nullptr);
    GDALClose(dataset);

    delete[] denormalizedHeights;

    if (writeLowDepth) {
        std::cout << "Writing second GeoTIFF with reduced depth..." << std::endl;

        auto heightsLowDepth = new int[resolutionX * resolutionY];
        arrayIndex = 0ul;
        /*
         * This flips the image vertically. See explanation above.
         */
        for (auto y = map->resolutionY; y > 0ul; y--) {
            for (auto x = 0ul; x < map->resolutionX; x++) {
                heightsLowDepth[arrayIndex++] = (int) (map->heights.at(calculate1DCoordinate(map, x, y - 1)) * (double) 255);
            }
        }

        dataset = driver->Create((destinationDEM + "_lowDepth.tiff").c_str(), resolutionX, resolutionY, 1, GDT_Byte, gdalDriverOptions);
        rasterBand = dataset->GetRasterBand(1);
        (void)! rasterBand->RasterIO(GF_Write, 0, 0, resolutionX, resolutionY, heightsLowDepth, resolutionX, resolutionY, GDT_Byte, 4, 4 * resolutionX, nullptr);
        GDALClose(dataset);

        delete[] heightsLowDepth;
    }
}

void
GTiffWriter::writeRGB(std::vector<std::vector<int>> data, int resolutionX, int resolutionY) {
    std::cout << "Writing RGB tiff..." << std::endl;

    int temp;
    for (auto y = 0; y < resolutionY / 2; y++) {
        for (auto x = 0; x < resolutionX; x++) {
            for (auto channel = 0; channel < 3; channel++) {
                temp = data.at(channel).at(x + y * resolutionX);
                data.at(channel).at(x + y * resolutionX) = data.at(channel).at(x + (resolutionY - 1 - y) * resolutionX);
                data.at(channel).at(x + (resolutionY - 1 - y) * resolutionX) = temp;
            }
        }
    }

    GDALRegister_GTiff();

    auto driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (driver == nullptr) { std::cout << "Could not get driver!" << std::endl; exit(Pipeline::EXIT_IO_ERROR); }
    if (!CSLFetchBoolean(GDALGetMetadata(driver, nullptr), GDAL_DCAP_CREATE, FALSE)) { std::cout << "Driver does not support creation!" << std::endl; exit(Pipeline::EXIT_IO_ERROR); }

    char** gdalDriverOptions = nullptr;
    gdalDriverOptions = CSLAddNameValue(gdalDriverOptions, "COMPRESS", compressionAlgorithm.c_str());
    gdalDriverOptions = CSLAddNameValue(gdalDriverOptions, "NUM_THREADS", "16");

    auto dataset = driver->Create((destinationDEM + ".tiff").c_str(), resolutionX, resolutionY, 3, GDT_Byte,gdalDriverOptions);
    if (dataset == nullptr) {
        std::cout << "WARNING :: GeoTIFF dataset could not be created at " << destinationDEM << ".tiff!" << std::endl;
        return;
    }
    GDALRasterBand * rasterBand;
    for (auto i = 1; i <= 3; i++) {
        rasterBand = dataset->GetRasterBand(i);
        (void) !rasterBand->RasterIO(GF_Write, 0, 0, resolutionX, resolutionY, (void *) data.at(i - 1).data(), resolutionX,
                                     resolutionY, GDT_Byte, sizeof(int), (long long) sizeof(int) * resolutionX, nullptr);
    }
    GDALClose(dataset);
}