cmake_minimum_required(VERSION 3.25)
project(EFTDEM)
set(CMAKE_CXX_STANDARD 20)

add_executable(EFTDEM FileIO.cpp FileIO.h main.cpp DataStructures.cpp Rasterizer.cpp Rasterizer.h GLHandler.cpp GLHandler.h Filler.cpp Filler.h Pipeline.h ICloudSorter.h ICloudReader.h ICloudRasterizer.h IHeightMapFiller.h IHeightMapWriter.h Pipeline.cpp IPipelineComponent.h)

find_package(GDAL CONFIG REQUIRED)

find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glbinding CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

#GTiff
target_link_libraries(EFTDEM PRIVATE GDAL::GDAL)

#OpenGL
target_link_libraries(EFTDEM PRIVATE glbinding::glbinding glbinding::glbinding-aux OpenGL::GL glfw glm::glm)
target_compile_definitions(EFTDEM PRIVATE ${DEFAULT_COMPILE_DEFINITIONS} GLFW_INCLUDE_NONE)