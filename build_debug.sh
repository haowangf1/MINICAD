rm -rf build
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH="D:/QT/6.10.1/msvc2022_64"  -DOpenCASCADE_DIR="D:/occt-install/occt-7.6.0-vc143/cmake" -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release


rm -rf build
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH="D:/QT/6.10.1/msvc2022_64"  -DOpenCASCADE_DIR="D:/occt-install/occt-7.6.0-vc143/cmake" -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --config Debug
