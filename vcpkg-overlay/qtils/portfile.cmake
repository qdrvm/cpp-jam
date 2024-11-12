vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 3aaf76e9e054763e28ec2978089acc3ec3101faf
  SHA512 072da0721be6c57d24f8bd412b64df07fc8efec5856cc8df9c8fca555de75f3b97f1dd2a29b6e5cba15f0af5b42cc8eb55937b1de770e015a56d4e2715b12528
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
    -DHUNTER_ENABLED=OFF)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
