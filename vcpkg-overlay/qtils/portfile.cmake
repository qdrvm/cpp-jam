vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 665971b1774b0b3e6b0bdba864480e85a5c7d1d5
  SHA512 23f443e48c66d8c3a20c419597c24717ef330f967b81ba3c7ad90575c53a6e321cd7d5ecd629ce772b1852f72c43a0b2e3d4a0ab1f7776a8ec9c7294b82b3c3d
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
