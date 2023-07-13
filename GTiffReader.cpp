#include "GTiffReader.h"
#include "Pipeline.h"
#include <iostream>
#include <gdal_priv.h>

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
heightMap * GTiffReader::apply(bool generateOutput) {

    std::cout << "Reading GeoTIFF..." << std::endl;

    if (!generateOutput) return nullptr;

    int resolutionX = (int) 16, resolutionY = (int) 16;

    GDALRegister_GTiff();

    auto driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (driver == nullptr) { std::cout << "Could not get driver!" << std::endl; exit(Pipeline::EXIT_IO_ERROR); }
    //if (!CSLFetchBoolean(GDALGetMetadata(driver, nullptr), GDAL_DCAP_CREATE, FALSE)) { std::cout << "Driver does not support creation!" << std::endl; exit(Pipeline::EXIT_IO_ERROR); }

    //char** gdalDriverOptions = nullptr;
    //gdalDriverOptions = CSLAddNameValue(gdalDriverOptions, "COMPRESS", "LZW");
    //gdalDriverOptions = CSLAddNameValue(gdalDriverOptions, "NUM_THREADS", "16");

    GDALAllRegister();//!!!!!!!!!! GDALRegister_GTiff();
    auto dataset = (GDALDataset *) GDALOpen(sourceDEM.c_str(), GA_ReadOnly);

    resolutionX = dataset->GetRasterXSize();
    resolutionY = dataset->GetRasterYSize();

    double * denormalizedHeights;



    /*GDALDataset * dataset = GDALOpen((sourceDEM + ".tiff").c_str(),GA_ReadOnly);// driver->pfnOpen((sourceDEM + ".tiff").c_str(), resolutionX, resolutionY, 1, GDT_Float64, gdalDriverOptions);
    auto rasterBand = dataset->GetRasterBand(1);
    (void)! rasterBand->RasterIO(GF_Read, 0, 0, resolutionX, resolutionY, denormalizedHeights, resolutionX, resolutionY, GDT_Float64, 0, 0, nullptr);
    GDALClose(dataset);

    double * normalizedHeights;
    double minX, minY, maxX, maxY;

    for (int i = 0; i < resolutionX * resolutionY; i++){
        minX = std::min(minX, denormalizedHeights[i]);
        minY = std::max(minY, denormalizedHeights[i]);
        maxX = std::max(maxX, denormalizedHeights[i]);
        maxY = std::max(maxY, denormalizedHeights[i]);
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