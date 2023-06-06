$depFolder=".deps"

if (Test-Path "build"){
    Remove-Item "build" -Recurse
}

cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="${depFolder}\vcpkg\scripts\buildsystems\vcpkg.cmake" -Dglfw3_DIR="${depFolder}\glfw\lib\cmake\glfw3" -Dglbinding_DIR="${depFolder}\glbinding" -Dglm_DIR="${depFolder}\glm\lib\cmake\glm" -G "Visual Studio 17 2022" -A x64

New-Item -Path "build\Debug" -ItemType Directory
New-Item -Path "build\Release" -ItemType Directory
Copy-Item -Path ".deps\glfw\bin\glfw3.dll" -Destination "build\Debug\glfw3.dll"
Copy-Item -Path ".deps\glfw\bin\glfw3.dll" -Destination "build\Release\glfw3.dll"
Copy-Item -Path ".deps\glbinding\glbindingd.dll" -Destination "build\Debug\glbindingd.dll"
Copy-Item -Path ".deps\glbinding\glbinding-auxd.dll" -Destination "build\Debug\glbinding-auxd.dll"
Copy-Item -Path ".deps\glbinding\glbinding.dll" -Destination "build\Release\glbinding.dll"
Copy-Item -Path ".deps\glbinding\glbinding-aux.dll" -Destination "build\Release\glbinding-aux.dll"