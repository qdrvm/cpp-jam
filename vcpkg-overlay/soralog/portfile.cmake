vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/soralog
  REF refs/tags/v0.2.5
  SHA512 ce98f52bbca0ee81c672b5a7fee5846f47c39eed4a9dfdeb91a6670c3cbfc2f96df23cf87f84216847d20be7bc52e54fa98ade0b5b7ca832d4ac1a7560396449
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
#vcpkg_cmake_config_fixup(PACKAGE_NAME "soralog")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
