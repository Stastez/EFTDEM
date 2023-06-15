cmake_minimum_required(VERSION 3.25)
project(EFTDEM)
set(CMAKE_CXX_STANDARD 20)

add_executable(EFTDEM main.cpp DataStructures.cpp GLHandler.cpp GLHandler.h ClosingFilter.cpp ClosingFilter.h Pipeline.h ICloudSorter.h ICloudReader.h ICloudRasterizer.h IHeightMapFiller.h IHeightMapWriter.h Pipeline.cpp IPipelineComponent.h CloudReader.cpp CloudReader.h GTiffWriter.cpp GTiffWriter.h SorterCPU.cpp SorterCPU.h RasterizerCpu.cpp RasterizerCpu.h RasterizerGpu.cpp RasterizerGpu.h SorterGpu.cpp SorterGpu.h ConfigProvider.cpp ConfigProvider.h)

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