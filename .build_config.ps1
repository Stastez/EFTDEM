$depFolder=".deps"

if (Test-Path "build"){
    Remove-Item "build" -Recurse
}

cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="${depFolder}\vcpkg\scripts\buildsystems\vcpkg.cmake" -Dglfw3_DIR="${depFolder}\glfw\lib\cmake\glfw3" -Dglbinding_DIR="${depFolder}\glbinding" -Dglm_DIR="${depFolder}\glm\lib\cmake\glm" -G "Visual Studio 17 2022" -A x64