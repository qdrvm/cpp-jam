vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/scale-codec-cpp
  REF 09f1ce9ffa75a50d48ea664757506ba7bd08d5c9
  SHA512 3197dde041e8e1df2f98c413f7aab9999321b948128b8bea2f45b3a648fbedc45b0ddcc93779606f09bf813cb59dac71e4ad2426ce4c0e19f73b7d8e1aa8b454
)
vcpkg_cmake_configure(
  SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
    -DJAM_COMPATIBLE=ON
    -DCUSTOM_CONFIG_SUPPORT=ON
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "scale")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
