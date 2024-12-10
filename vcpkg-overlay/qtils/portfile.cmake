vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF e0e3914ef4e2e7c3c074aa39f9e9cd27cb72c032
  SHA512 b5b9c33c1925fdb0c58f9a81df3d23604000f1e1c914411d90498f8768f0ae49271cce255e98e17fc909a0d0f2cacd156fdf9b7409216f7922a26dedace552b1
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
