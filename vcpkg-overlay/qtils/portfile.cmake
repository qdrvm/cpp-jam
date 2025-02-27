vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 6d02006f7c0fe44958acbfe53016fa81f1ac0bf5
  SHA512 3e46c767bb30a5e6d1df38e7a4c6fb5ec61c320bbbf6bbe622043402119216a8f7ddc1e6841a26c8848c20f38e1dfbd37f2b505698802e3a95ace464bb8c2cb9
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
