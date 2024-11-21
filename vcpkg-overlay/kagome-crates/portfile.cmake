vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/kagome-crates
  REF 77f9e098a66e6c622e52f415b296ffa14e63603a
  SHA512 6c3fddddf5be0f3ed8e5912d82fc0c97ac1f65d402e0c773149983b43b694d1d4b3b4bdc7af923bd3d2f94d53737f35d996275682bb366842870999b5b243138
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "kagome-crates")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
