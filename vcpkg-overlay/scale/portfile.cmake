vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/scale-codec-cpp
  REF refs/tags/v2.0.1
  SHA512 cc5158cdfecea7f516e2f110adcc00f3024649507908b366ef34a76398efcb735011d8fcacd2f41016eb769dfa74e93ee3af20049560fafd1576b280fb92acc7
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
