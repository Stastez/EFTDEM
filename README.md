# Tabp Efficient Filling Techniques

## Installation

### Windows

Just run [build_deps.bat](./build_config.bat). This will create a PowerShell instance that bypasses script execution policies and runs [.build_deps.ps1](./.build_deps.ps1).
This will clone all necessary dependencies from GitHub and compile them into a directory called .deps. GDAL will be installed via vcpkg.

## Dependencies

[GDAL](https://gdal.org/)  
[glfw](https://www.glfw.org/)  
[glm](https://github.com/g-truc/glm)  
[glbinding](https://github.com/cginternals/glbinding)  