vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/scale-codec-cpp
  REF 6ecc0c67eadacc9ab8acdeb73488bdc4c6e30d1c
  SHA512 22267674113f04fb9c4624bd2cb69e6f4053edd129bfa5ffb83136e4ce2c04f6fe3dce9c729d5df08c84fd92a8a69938c249fa5068132ebc4a42d205142fb921
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
