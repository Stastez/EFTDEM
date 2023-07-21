# Tabp Efficient Filling Techniques

## Usage
### Simple filling
Provide exactly one .yaml config of the type presented below as a command line argument.

### Comparison between our filling algorithms and a ground-truth DEM
Provide exactly one .tiff (must be tiff, not tif) followed by exactly one .yaml config of the type presented below as a command line argument.

### Comparison between several layers of ground radar
Provide at least two .yaml configs of the type presented below.

## Example Config
```yaml
CloudReaderOptions:
  pointCloudPath: "../resources/pointCloud.txt"

  # Currently implemented: mobileMapping, groundRadar
  pointCloudType: mobileMapping

CloudSorterOptions:
  pixelPerUnit: 5
  #pixelPerUnitX: 5 ### optional, remove pixelPerUnit if you want to use this
  #pixelPerUnitY: 5 ### optional, remove pixelPerUnit if you want to use this
  useGPU: true

CloudRasterizerOptions:
  useGPU: true

HeightMapFillerOptions:
  useGPU: true

  # Currently implemented: closingFilter, inverseDistanceWeightedFilter, radialFiller, dummy (use dummy to output the unfilled rasterized version)
  filler: inverseDistanceWeightedFilter

  # for closingFilter and inverseDistanceWeightedFilter
  kernelBasedFilterOptions:
    kernelSizes:
      - 5
      - 25
      - 50
    batchSize: 1024 # optional, if not set will default to 0 (automatically trying to find possible batch size)

  # for radialFiller
  radialFillerOptions:
    maxHoleRadius: 50

HeightMapWriterOptions:
  destinationPath: "../resources/map"
  writeLowDepth: true
  betterCompression: false # optional, will default to "false". If true, GDAL will use LZMA compression, if false, LZW.
                           # Using LZMA will reduce file size significantly, however, the export will take a much extended
                           # amount of time. Also, using LZMA max break compatibility with standard image viewers. You may
                           # use software such as QGIS to mitigate this issue.

OpenGLOptions:
  shaderDirectory: "../shaders"

# for layer-wise comparison
ComparisonOptions:
  destinationPath: "../resources/"
  threshold: 8.336743E-09
```

## Hardware Requirements
- An OpenGL 4.3 or greater capable combination of GPU and graphics driver
- Lots of RAM
  - EFTDEM uses 64 bit doubles for the values of points and height data
  - Optimization of computation time requires lots of (shared) GPU memory

## Installation

You may choose to provide a built version of all dependencies listed below yourself. For added convenience,
we also provide installation scripts that fetch and build these dependencies for you. As a last step,
these scripts also set up a valid CMake project for EFTDEM while copying all necessary
shared libraries. For all systems, these scripts require CMake and git.

### Windows

You may run [build_deps.bat](./build_deps.bat). The installation script requires CMake to be installed in the system PATH,
git, PowerShell and Visual Studio 2022.
Running the script will create a PowerShell instance that bypasses script execution policies and run [.build_deps.ps1](./.build_deps.ps1).
This will clone all necessary dependencies from GitHub and compile them into a directory called `.deps`. GDAL will be installed via vcpkg.
Lastly, the script will execute [.build_config.ps1](./.build_config.ps1) which will set up a valid CMake project for
EFTDEM with the build-directory specified as `build`.

### Linux (X11)

You may run [build_deps.sh](./build_deps.sh). The installation script requires CMake and git to be to be installed in the system PATH,
as well as a working C++ compiler.
The script will clone all necessary dependencies from GitHub and compile them into a directory called "deps". GDAL will be installed via vcpkg.
Lastly, the script will execute [build_config.sh](./build_config.sh) which will set up a valid CMake project for
EFTDEM with the build-directories specified as `build-debug`, `build-debug-coverage` and `build-release`.

## Dependencies

[GDAL](https://gdal.org/)  
[glfw](https://www.glfw.org/)  
[glm](https://github.com/g-truc/glm)  
[glbinding](https://github.com/cginternals/glbinding)  
[magic_enum](https://github.com/Neargye/magic_enum)  
[yaml-cpp](https://github.com/jbeder/yaml-cpp)  
OpenGL

## Known Errors

- When executing shaders on the GPU, Windows may terminate the graphics driver when a single compute invocations exceeds
a certain time limit (usually 2 seconds). This may lead to a program crash or visual artifacts in the resulting output.
  - **Fix:** Specify a smaller `batchSize` in your config or set `batchSize=0` to trigger automatic testing for a good batch size
    (note: automatic testing may conclude with batch sizes that are smaller than technically possible. This leads to decreased performance.
    In rare cases automatic testing may also still choose batch sizes that are too large.)
- EFTDEM may crash in either scenario:
  - Amount of points in the specified point cloud file is too great for `std::vector` allocation with your current setup
    - **Fix:** Tile the data into multiple files and run EFTDEM on them seperately.
  - Resolution specified in the config is too great for GPU buffers or `std::vector` allocations
    - **Fix:** Reduce either `pixelPerUnitX` or `pixelPerUnitY`, or both. You may also tile your data as described above.
    In that case, make sure to sort the point cloud file in at least the `x` or `y` axis before proceeding with the tiling
    operation.