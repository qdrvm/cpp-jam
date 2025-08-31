vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/leanp2p
  REF 7cd8567a07e8b9990766c61253aabe6b0d656d04
  SHA512 a4aa2a5a28fbe16a7ba153e49f6d6c161ae1cfbca2ec0ec04d1524efbdcb0fdb5c6d251fe06b464a534d18a44a6783eeaac085f36db3c74d53fe5fdc7b4de8c8
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
