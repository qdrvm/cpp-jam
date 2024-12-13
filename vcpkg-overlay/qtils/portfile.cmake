vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 9796aad4181553f738d754c3cc4a779e32fa389b
  SHA512 ad45fcb1e78f3fb55f8826be016558edf7cd984b2c9327fa566f4f1d8b5db4914fb14d791e3d6b8813d7ec2f2b80d8471d149576134c962a1138374ed68bf69d
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
