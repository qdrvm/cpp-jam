vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF ad6c66c18c9e6d50491c42ac74e879f5a5176f51
  SHA512 407619b1aedce2290de0ae90c2363b15fc407ca975ef5df032f449a0d8f9d3c2ac12aef4d2c472237c974164cda4a4ddff874dbe6ef6871114d727d499c399ec
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
