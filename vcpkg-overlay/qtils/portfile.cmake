vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 837b925296b3824d9fd6b46104990dc56671a1a7
  SHA512 c84c82b77797fd84c1970371edc00029b784ee73682dea31503fc4f03be8f9474b1387a68ad595c217f53fa61fa2398e17014c2e6e6b1b0764bd687e0a4a3d2a
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
    -DHUNTER_ENABLED=OFF)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
