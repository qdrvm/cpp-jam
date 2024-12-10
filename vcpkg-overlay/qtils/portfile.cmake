vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 07a66bdcc10b3707ffcaf708a9625f8f9116cd16
  SHA512 d1ce7fff3a115d9c2ff34f4ec221ab835aa8a1f90304f69e568be027bfb2a2bf0ba247e8bd9e3a85041e9d89720b31447eac1c892d7b35b5016f9c16207a7d4b
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
