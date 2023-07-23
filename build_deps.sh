depFolder="deps"
removeOld=true
numThreads="${nproc}"

mkdir "$depFolder"

#get glfw, glbinding, magic_enum, yaml-cpp
cd "$depFolder"
git clone https://github.com/glfw/glfw.git
git clone https://github.com/cginternals/glbinding.git
git clone https://github.com/Neargye/magic_enum.git
git clone https://github.com/jbeder/yaml-cpp.git

#build glfw
if [ -d "glfw/build-debug" ] && $removeOld; then
    rm -rf "glfw/build-debug"
fi
if [ -d "glfw/build-release" ] && $removeOld; then
    rm -rf "glfw/build-release"
fi

cmake   -B "glfw/build-debug" \
        -S "glfw" \
        -D CMAKE_BUILD_TYPE="Debug" \
        -D CMAKE_INSTALL_PREFIX="glfw" \
        -D BUILD_SHARED_LIBS=ON \
        -D GLFW_BUILD_DOCS=OFF \
        -D GLFW_BUILD_EXAMPLES=OFF \
        -D GLFW_BUILD_TESTS=OFF \
        -D GLFW_BUILD_X11=ON \
        -D GLFW_BUILD_WAYLAND=OFF \
        -D GLFW_INSTALL=ON \
        -G "Unix Makefiles"

cmake --build "glfw/build-debug" -j $numThreads --target all
cmake --build "glfw/build-debug" -j $numThreads --target install

cmake   -B "glfw/build-release" \
        -S "glfw" \
        -D CMAKE_BUILD_TYPE="Release" \
        -D CMAKE_INSTALL_PREFIX="glfw" \
        -D BUILD_SHARED_LIBS=ON \
        -D GLFW_BUILD_DOCS=OFF \
        -D GLFW_BUILD_EXAMPLES=OFF \
        -D GLFW_BUILD_TESTS=OFF \
        -D GLFW_BUILD_X11=ON \
        -D GLFW_BUILD_WAYLAND=OFF \
        -D GLFW_INSTALL=ON \
        -G "Unix Makefiles"

cmake --build "glfw/build-release" -j $numThreads --target all
cmake --build "glfw/build-release" -j $numThreads --target install

#build glbinding
if [ -d "glbinding/build-debug" ] && $removeOld; then
    rm -rf "glbinding/build-debug"
fi
if [ -d "glbinding/build-release" ] && $removeOld; then
    rm -rf "glbinding/build-release"
fi

cmake   -B "glbinding/build-debug" \
        -S "glbinding" \
        -D CMAKE_BUILD_TYPE="Debug" \
        -D CMAKE_INSTALL_PREFIX="glbinding" \
        -D BUILD_SHARED_LIBS=ON \
        -D ENABLE_WIN_INSTALL_HACKS=OFF \
        -D OPTION_BUILD_CHECK=OFF \
        -D OPTION_BUILD_DOCS=OFF \
        -D OPTION_BUILD_EXAMPLES=OFF \
        -D OPTION_BUILD_OWN_KHR_HEADERS=ON \
        -D OPTION_BUILD_TESTS=OFF \
        -D OPTION_BUILD_TOOLS=OFF \
        -G "Unix Makefiles"

cmake --build "glbinding/build-debug" -j $numThreads --target all
cmake --build "glbinding/build-debug" -j $numThreads --target install

cmake   -B "glbinding/build-release" \
        -S "glbinding" \
        -D CMAKE_BUILD_TYPE="Release" \
        -D CMAKE_INSTALL_PREFIX="glbinding" \
        -D BUILD_SHARED_LIBS=ON \
        -D ENABLE_WIN_INSTALL_HACKS=OFF \
        -D OPTION_BUILD_CHECK=OFF \
        -D OPTION_BUILD_DOCS=OFF \
        -D OPTION_BUILD_EXAMPLES=OFF \
        -D OPTION_BUILD_OWN_KHR_HEADERS=ON \
        -D OPTION_BUILD_TESTS=OFF \
        -D OPTION_BUILD_TOOLS=OFF \
        -G "Unix Makefiles"

cmake --build "glbinding/build-release" -j $numThreads --target all
cmake --build "glbinding/build-release" -j $numThreads --target install

#build magic_enum
if [ -d "magic_enum/build-debug" ] && $removeOld; then
    rm -rf "magic_enum/build-debug"
fi
if [ -d "magic_enum/build-release" ] && $removeOld; then
    rm -rf "magic_enum/build-release"
fi

cmake   -B "magic_enum/build-debug" \
        -S "magic_enum" \
        -D CMAKE_BUILD_TYPE="Debug" \
        -D CMAKE_INSTALL_PREFIX="magic_enum" \
        -D MAGIC_ENUM_OPT_BUILD_EXAMPLES=OFF \
        -D MAGIC_ENUM_OPT_BUILD_TESTS=OFF \
        -D MAGIC_ENUM_OPT_INSTALL=ON \
        -G "Unix Makefiles"

cmake --build "magic_enum/build-debug" -j $numThreads --target all
cmake --build "magic_enum/build-debug" -j $numThreads --target install

cmake   -B "magic_enum/build-release" \
        -S "magic_enum" \
        -D CMAKE_BUILD_TYPE="Release" \
        -D CMAKE_INSTALL_PREFIX="magic_enum" \
        -D MAGIC_ENUM_OPT_BUILD_EXAMPLES=OFF \
        -D MAGIC_ENUM_OPT_BUILD_TESTS=OFF \
        -D MAGIC_ENUM_OPT_INSTALL=ON \
        -G "Unix Makefiles"

cmake --build "magic_enum/build-release" -j $numThreads --target all
cmake --build "magic_enum/build-release" -j $numThreads --target install

#build yaml-cpp
if [ -d "yaml-cpp/build-debug" ] && $removeOld; then
    rm -rf "yaml-cpp/build-debug"
fi
if [ -d "yaml-cpp/build-release" ] && $removeOld; then
    rm -rf "yaml-cpp/build-release"
fi

cmake   -B "yaml-cpp/build-debug" \
        -S "yaml-cpp" \
        -D CMAKE_BUILD_TYPE="Debug" \
        -D CMAKE_INSTALL_PREFIX="yaml-cpp" \
        -D BUILD_TESTING=OFF \
        -D BUILD_SHARED_LIBS=ON \
        -D YAML_CPP_INSTALL=ON \
        -G "Unix Makefiles"

cmake --build "yaml-cpp/build-debug" -j $numThreads --target all
cmake --build "yaml-cpp/build-debug" -j $numThreads --target install

cmake   -B "yaml-cpp/build-release" \
        -S "yaml-cpp" \
        -D CMAKE_BUILD_TYPE="Release" \
        -D CMAKE_INSTALL_PREFIX="yaml-cpp" \
        -D BUILD_TESTING=OFF \
        -D BUILD_SHARED_LIBS=ON \
        -D YAML_CPP_INSTALL=ON \
        -G "Unix Makefiles"

cmake --build "yaml-cpp/build-release" -j $numThreads --target all
cmake --build "yaml-cpp/build-release" -j $numThreads --target install

#build GDAL
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg install gdal --triplet=x64-linux
./vcpkg integrate install

cd "../.."
./build_config.sh