vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/scale-codec-cpp
  REF fb0a319eb7eb79f0ef8c1e01088daa489042d04d
  SHA512 fea9aec94fef53ccf84854dccc57bde74f8eddfbcfd03abed30252944d610bf84581be7176541c2e3d20c5ac99e2965e2959ed13296c0ce123c778d4017ea660
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
