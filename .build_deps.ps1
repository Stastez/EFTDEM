$depFolder=".deps"
$removeOld=$true

if (-Not (Test-Path ".deps")){
    New-Item -Path ".deps" -ItemType Directory
}

#get glfw, glbinding, glm, magic_enum
Set-Location "${depFolder}"
git clone https://github.com/glfw/glfw.git
git clone https://github.com/cginternals/glbinding.git
git clone https://github.com/g-truc/glm.git
git clone https://github.com/Neargye/magic_enum.git

#build glfw
if ((Test-Path "glfw\build") -and $removeOld){
    Remove-Item "glfw\build" -Recurse
}
cmake -B "glfw\build" -S "glfw" -DCMAKE_CONFIGURATION_TYPES="Debug;Release" -DCMAKE_INSTALL_PREFIX="glfw" -DBUILD_SHARED_LIBS=ON -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -G "Visual Studio 17 2022" -A x64
cmake --build "glfw\build" --target ALL_BUILD --config Debug
cmake --build "glfw\build" --target ALL_BUILD --config Release
cmake --build "glfw\build" --target INSTALL --config Debug
cmake --build "glfw\build" --target INSTALL --config Release

#build glm
if ((Test-Path "glm\build") -and $removeOld){
    Remove-Item "glm\build" -Recurse
}
cmake -B "glm\build" -S "glm" -D CMAKE_CONFIGURATION_TYPES="Debug;Release" -D CMAKE_INSTALL_PREFIX="glm" -D BUILD_SHARED_LIBS:BOOL=ON -D BUILD_STATIC_LIBS:BOOL=ON -D BUILD_TESTING:BOOL=OFF -G "Visual Studio 17 2022" -A x64
cmake --build "glm\build" --target ALL_BUILD --config Debug
cmake --build "glm\build" --target ALL_BUILD --config Release
cmake --build "glm\build" --target INSTALL --config Debug
cmake --build "glm\build" --target INSTALL --config Release

#build glbinding
if ((Test-Path "glbinding\build") -and $removeOld){
    Remove-Item "glbinding\build" -Recurse
}
cmake -B "glbinding\build" -S "glbinding" -DCMAKE_CONFIGURATION_TYPES="Debug;Release" -DCMAKE_INSTALL_PREFIX="glbinding" -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS:BOOL=OFF -DOPTION_BUILD_EXAMPLES=OFF -DOPTION_BUILD_OWN_KHR_HEADERS=ON -Dglfw3_DIR="glfw\lib\cmake\glfw3" -G "Visual Studio 17 2022" -A x64
cmake --build "glbinding\build" --target ALL_BUILD --config Debug
cmake --build "glbinding\build" --target ALL_BUILD --config Release
cmake --build "glbinding\build" --target INSTALL --config Debug
cmake --build "glbinding\build" --target INSTALL --config Release

#build magic_enum
if ((Test-Path "magic_enum\build") -and $removeOld){
    Remove-Item "magic_enum\build" -Recurse
}
cmake -B "magic_enum\build" -S "magic_enum" -D CMAKE_CONFIGURATION_TYPES="Debug;Release" -D CMAKE_INSTALL_PREFIX="magic_enum" -D MAGIC_ENUM_OPT_BUILD_EXAMPLES=OFF -D MAGIC_ENUM_OPT_BUILD_TESTS=OFF -D MAGIC_ENUM_OPT_INSTALL=ON
cmake --build "magic_enum\build" --target ALL_BUILD --config Debug
cmake --build "magic_enum\build" --target ALL_BUILD --config Release
cmake --build "magic_enum\build" --target INSTALL --config Debug
cmake --build "magic_enum\build" --target INSTALL --config Release

git clone https://github.com/Microsoft/vcpkg.git
Set-Location "vcpkg"
.\bootstrap-vcpkg.bat
.\vcpkg.exe install gdal --triplet=x64-windows
.\vcpkg.exe integrate install

Set-Location "..\.."
& "$PSScriptRoot/.build_config.ps1"