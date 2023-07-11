cmake_minimum_required(VERSION 3.25)
project(EFTDEM)
set(CMAKE_CXX_STANDARD 20)

add_executable(EFTDEM
        main.cpp
        DataStructures.cpp
        GLHandler.cpp
        GLHandler.h
        ClosingFilter.cpp
        ClosingFilter.h
        Pipeline.h
        ICloudSorter.h
        ICloudReader.h
        ICloudRasterizer.h
        IHeightMapFiller.h
        IHeightMapWriter.h
        Pipeline.cpp
        IPipelineComponent.h
        MobileMappingReader.cpp
        MobileMappingReader.h
        GTiffWriter.cpp
        GTiffWriter.h
        SorterCPU.cpp
        SorterCPU.h
        RasterizerCpu.cpp
        RasterizerCpu.h
        RasterizerGpu.cpp
        RasterizerGpu.h
        SorterGpu.cpp
        SorterGpu.h
        IKernelBasedFilter.h
        IKernelBasedFilter.cpp
        InverseDistanceWeightedFilter.h
        InverseDistanceWeightedFilter.cpp
        ConfigProvider.cpp
        ConfigProvider.h
        GroundRadarReader.cpp
        GroundRadarReader.h
        FillerLoop.cpp
        FillerLoop.h
        ClosingFilter.cpp
        ClosingFilter.h
        RadarComparator.cpp
        RadarComparator.h
        TiffPipeline.cpp
        TiffPipeline.h
        GroundTruthComparator.cpp
        GroundTruthComparator.h
        IComparator.cpp
        IComparator.h
        IPipeline.cpp
        IPipeline.h
        GTiffReader.cpp
        RadialDilator.cpp
        RadialDilator.h
        IRadialFillerComponent.h
        RadialBufferSwapper.cpp
        RadialBufferSwapper.h
        RadialEroder.cpp
        RadialEroder.h
        RadialFiller.cpp
        RadialFiller.h)

if(MSVC)
    target_compile_options(EFTDEM PRIVATE /W4)
else()
    target_compile_options(EFTDEM PRIVATE -Wall -Wextra -Wpedantic)
endif()

find_package(GDAL CONFIG REQUIRED)

find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glbinding CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

find_package(magic_enum CONFIG REQUIRED)

find_package(yaml-cpp CONFIG REQUIRED)

#GTiff
target_link_libraries(EFTDEM PRIVATE GDAL::GDAL)

#OpenGL
target_link_libraries(EFTDEM PRIVATE glbinding::glbinding glbinding::glbinding-aux OpenGL::GL glfw glm::glm)
target_compile_definitions(EFTDEM PRIVATE ${DEFAULT_COMPILE_DEFINITIONS} GLFW_INCLUDE_NONE)

#magic_enum
target_link_libraries(EFTDEM PRIVATE magic_enum::magic_enum)

#yaml-cpp
target_link_libraries(EFTDEM PRIVATE yaml-cpp)