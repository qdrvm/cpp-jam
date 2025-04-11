vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 629ad648233a857d898efdcdb0f681ef5c299947
  SHA512 40f41564c6d2f031a126730db3a34c6b7217443ed0551b7baf41e915c3a0e6670db55b656b6478abe14992fd049e0b31fafbd4ae0eb1b29c93c2782ca27e1c13
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
    -DHUNTER_ENABLED=OFF)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
