vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/scale-codec-cpp
  REF 6d80d0f4aa52c86cf2875c77a94583a0a4069626
  SHA512 0472caa6804f5ee693c6de577e78fc07a77d12aabe061e4ac9dce4957b18e54a8814cf3555adbaa4bf54f9c0c9023c28be7fdac54ac511006a2b98bedac30f30
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
