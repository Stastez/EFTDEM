# Tabp Efficient Filling Techniques

## Installation

### Windows

Just run [build_deps.bat](./build_config.bat). The installation script requires CMake to be installed in the system PATH,
PowerShell and Visual Studio 2022.
Running the script will create a PowerShell instance that bypasses script execution policies and run [.build_deps.ps1](./.build_deps.ps1).
This will clone all necessary dependencies from GitHub and compile them into a directory called .deps. GDAL will be installed via vcpkg.
In your IDE, switch the CMake generator to Visual Studio 17.

## Dependencies

[GDAL](https://gdal.org/)  
[glfw](https://www.glfw.org/)  
[glm](https://github.com/g-truc/glm)  
[glbinding](https://github.com/cginternals/glbinding)  
[magic_enum](https://github.com/Neargye/magic_enum)  