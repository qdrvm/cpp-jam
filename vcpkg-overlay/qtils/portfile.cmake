vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF b438b00a1e2bd96506bfac0921ea3891b9234004
  SHA512 07db1ab91fafb1e4c2692b0c473727b906e1860f82b6b98e2f2c1d2742bb27a393fa79dc85afab2ac4ed0fc97e889dba6da0422f932f46d5336c31947aa5825f
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
