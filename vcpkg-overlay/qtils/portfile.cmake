vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
#  REF refs/tags/v0.1.0
  REF 7fa095bed51f9c1b7d4a2c99552229df380373ea
  SHA512 b03d6d324c28b57f20d23ae17be1e61fddf6d394c2b6fd5fff886d36018a12bd83593e25718e9d7f1ed88f8fb3b6ab71b5fb7c7b3f2f208493c8d16b22a0af2a
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
