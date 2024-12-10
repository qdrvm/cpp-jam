vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/scale-codec-cpp
  REF 7a5e8a7802f8a9339cf44679252a10c65a0f5a25
  SHA512 bf8aa4f5066bb6ff5239f3bea9caecc6a5cfc44f9f3ff3eaf63d404b720ac0b50c7722db594792cecc4421e7035cce353d5faf87bcebdee9ab999c9f1759089b
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "scale")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
