vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF eb5910c1c85e83b1c782f93e4fc86f0de8100676
  SHA512 fb0d03bc625109fb39508b6aa0dd1d5958118d6699215b4688966fe52f1e86465e873cd178bedd335e2c4258e2ec59fce07eb949c17ed60199f1b0ef0e020ae6
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
    -DHUNTER_ENABLED=OFF)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
