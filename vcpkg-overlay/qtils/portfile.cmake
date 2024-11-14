vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 5f46bf03ec9af39c7a1bd85e5f96ccbaa9f4a064
  SHA512 848577aa4f9abd3d5f8154af9bdf01c81ecb4e41ceb2993e752831ce92706e2fce31fb67a414ee0e5c69424e896941f66877d582f6fd334c193dae365f9297d1
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
    -DHUNTER_ENABLED=OFF)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
