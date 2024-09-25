vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/kagome-crates
  REF 0688fec85a521eb8d19acb88fdfbb1ca33961538
  SHA512 565c785993835a23c89f6bf178cb3e9d11deedc19a910458dc8db237b1319018a4a99e7cae546c8faa0ed24413ed36df0521c329ae78598544b6144c44f8a3b0
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "kagome-crates")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
