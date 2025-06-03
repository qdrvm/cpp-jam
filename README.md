## Build
```cmake
rm -rf build && cmake -B build -DCMAKE_BUILD_TYPE=[Release | Debug] -DTESTING=ON -DCMAKE_TOOLCHAIN_FILE=<path to vcpkg.cmake> -G "Ninja" -DVCPKG_OVERLAY_PORTS=vcpkg-overlay
```
```cmake
cmake --build build
```