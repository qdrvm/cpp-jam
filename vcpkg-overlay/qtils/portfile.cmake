vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF bf0c6a780a274f4d0529995e576542dc59865abc
  SHA512 ed501898a9949eee17b982a659d223e7eb1a7ee929a33dfa5f36d59502d29864910873958f48b5b517a4ebddb07493e5c0adfb893a198c340ce20bf056732c04
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
