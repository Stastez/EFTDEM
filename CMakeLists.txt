cmake_minimum_required(VERSION 3.25)
project(EFTDEM)
set(CMAKE_CXX_STANDARD 20)

add_executable(EFTDEM fileIO.cpp fileIO.h main.cpp dataStructures.cpp rasterizer.cpp rasterizer.h glHandler.cpp glHandler.h)

find_package(GDAL CONFIG REQUIRED)

find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glbinding CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

find_package(OpenCL CONFIG REQUIRED)

#GTiff
target_link_libraries(EFTDEM PRIVATE GDAL::GDAL)

#OpenGL
target_link_libraries(EFTDEM PRIVATE glbinding::glbinding glbinding::glbinding-aux OpenGL::GL glfw glm::glm)
target_compile_definitions(EFTDEM PRIVATE ${DEFAULT_COMPILE_DEFINITIONS} GLFW_INCLUDE_NONE)

target_link_libraries(EFTDEM PRIVATE OpenCL::OpenCL)