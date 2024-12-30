vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF 0dd35029f54e9a6218ee261b0227ef23762f2d4b
  SHA512 fa749d2d489282073e10ef43487a8a98733ec071ebfebe665c8c9692568a15b5542689e4f23aeef5c1442ba6513d8c9563a22169bf0657a619ad01109a84ce0b
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
