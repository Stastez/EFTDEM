depFolder="deps"
debugFolder="build-debug"
coverageFolder="build-debug-coverage"
releaseFolder="build-release"

if [ -d "$depFolder" ]; then
    rm -rf "build"
fi

cmake   -B build-debug \
        -S . \
        -D CMAKE_TOOLCHAIN_FILE="$depFolder/vcpkg/scripts/buildsystems/vcpkg.cmake" \
        -D glfw3_DIR="$depFolder/glfw/lib/cmake/glfw3" \
        -D glbinding_DIR="$depFolder/glbinding" \
        -D glm_DIR="$depFolder/glm/lib/cmake/glm" \
        -D magic_enum_DIR="$depFolder/magic_enum/lib/cmake/magic_enum" \
        -D yaml-cpp_DIR="$depFolder/yaml-cpp/lib/cmake/yaml-cpp" \
        -G "Unix Makefiles"

cmake   -B build-debug-coverage \
        -S . \
        -D CMAKE_CXX_FLAGS=--coverage \
        -D CMAKE_C_FLAGS=--coverage \
        -D CMAKE_TOOLCHAIN_FILE="$depFolder/vcpkg/scripts/buildsystems/vcpkg.cmake" \
        -D glfw3_DIR="$depFolder/glfw/lib/cmake/glfw3" \
        -D glbinding_DIR="$depFolder/glbinding" \
        -D glm_DIR="$depFolder/glm/lib/cmake/glm" \
        -D magic_enum_DIR="$depFolder/magic_enum/lib/cmake/magic_enum" \
        -D yaml-cpp_DIR="$depFolder/yaml-cpp/lib/cmake/yaml-cpp" \
        -G "Unix Makefiles"

cmake   -B build-release \
        -S . \
        -D CMAKE_TOOLCHAIN_FILE="$depFolder/vcpkg/scripts/buildsystems/vcpkg.cmake" \
        -D glfw3_DIR="$depFolder/glfw/lib/cmake/glfw3" \
        -D glbinding_DIR="$depFolder/glbinding" \
        -D glm_DIR="$depFolder/glm/lib/cmake/glm" \
        -D magic_enum_DIR="$depFolder/magic_enum/lib/cmake/magic_enum" \
        -D yaml-cpp_DIR="$depFolder/yaml-cpp/lib/cmake/yaml-cpp" \
        -G "Unix Makefiles"

mkdir "$debugFolder"
mkdir "$coverageFolder"
mkdir "$releaseFolder"

cp "$depFolder/glfw/lib/libglfw.so" "$debugFolder"
cp "$depFolder/glfw/lib/libglfw.so" "$coverageFolder"
cp "$depFolder/glfw/lib/libglfw.so" "$releaseFolder"

cp "$depFolder/glm/build-debug/glm/libglm_shared.so" "$debugFolder"
cp "$depFolder/glm/build-debug/glm/libglm_shared.so" "$coverageFolder"
cp "$depFolder/glm/build-release/glm/libglm_shared.so" "$releaseFolder"

cp "$depFolder/glbinding/lib/libglbindingd.so" "$depFolder/glbinding/lib/libglbinding-auxd.so" -t "$debugFolder"
cp "$depFolder/glbinding/lib/libglbindingd.so" "$depFolder/glbinding/lib/libglbinding-auxd.so" -t "$coverageFolder"
cp "$depFolder/glbinding/lib/libglbinding.so" "$depFolder/glbinding/lib/libglbinding-aux.so" -t "$releaseFolder"

cp "$depFolder/yaml-cpp/lib/libyaml-cppd.so" "$debugFolder"
cp "$depFolder/yaml-cpp/lib/libyaml-cppd.so" "$coverageFolder"
cp "$depFolder/yaml-cpp/lib/libyaml-cpp.so" "$releaseFolder"