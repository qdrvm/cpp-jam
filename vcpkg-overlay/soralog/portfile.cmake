vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO xDimon/soralog
  REF c13d77dd67e8e380f1c16065c988416d53cadbeb
  SHA512 0a6e4ebc604e84895a092268240d73d39784bbafb4baae39ec51d720ee06931632c22c2f8724a6d7655967ddf273e3d22f2aa02e4b42b22a29c0dec21e0f424a
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
#vcpkg_cmake_config_fixup(PACKAGE_NAME "soralog")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
