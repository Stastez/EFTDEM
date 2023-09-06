$depFolder=".deps"

if (Test-Path "build"){
    Remove-Item "build" -Recurse
}

cmake   -B build -S . -D CMAKE_TOOLCHAIN_FILE="${depFolder}\vcpkg\scripts\buildsystems\vcpkg.cmake" -D glfw3_DIR="${depFolder}\glfw\lib\cmake\glfw3" -D glbinding_DIR="${depFolder}\glbinding" -D magic_enum_DIR="${depFolder}\magic_enum\lib\cmake\magic_enum" -D yaml-cpp_DIR="${depFolder}\yaml-cpp\lib\cmake\yaml-cpp" -G "Visual Studio 17 2022" -A x64

New-Item -Path "build\Debug" -ItemType Directory
New-Item -Path "build\Release" -ItemType Directory
Copy-Item -Path ".deps\glfw\bin\glfw3.dll" -Destination "build\Debug\glfw3.dll"
Copy-Item -Path ".deps\glfw\bin\glfw3.dll" -Destination "build\Release\glfw3.dll"
Copy-Item -Path ".deps\glbinding\glbindingd.dll" -Destination "build\Debug\glbindingd.dll"
Copy-Item -Path ".deps\glbinding\glbinding-auxd.dll" -Destination "build\Debug\glbinding-auxd.dll"
Copy-Item -Path ".deps\glbinding\glbinding.dll" -Destination "build\Release\glbinding.dll"
Copy-Item -Path ".deps\glbinding\glbinding-aux.dll" -Destination "build\Release\glbinding-aux.dll"
Copy-Item -Path ".deps\yaml-cpp\build\Debug\yaml-cppd.dll" -Destination "build\Debug\yaml-cppd.dll"
Copy-Item -Path ".deps\yaml-cpp\build\Release\yaml-cpp.dll" -Destination "build\Release\yaml-cpp.dll"