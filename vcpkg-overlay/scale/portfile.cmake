vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/scale-codec-cpp
  REF c59aa2b98eec0b16a6f224740d9c4c0de616818e
  SHA512 a4e3f18e8ade1e18fdd2df0791a47c62f856f0ea5dde663fd00c82dc71302e21b7fc99fd29392cd6f131d7ed263eafebccd2311c4f977297174b6332891a938e
)
vcpkg_cmake_configure(
  SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
    -DJAM_COMPATIBLE=ON
    -DCUSTOM_CONFIG_SUPPORT=ON
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "scale")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
