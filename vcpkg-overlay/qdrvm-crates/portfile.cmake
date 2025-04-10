vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/kagome-crates
  REF d49af753f5dbdd6733298fa696eb9b120ba2f8ef
  SHA512 6cb2075f96438d8f64a9622295e288f42ba2a3099aff44af949fab3bb25410b519d3511242d0a6bab2aaa3b6c154448e056a3ac7f908d0241417434cb856a327
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}" OPTIONS "-DQDRVM_BIND_CRATES=bandersnatch_vrfs;schnorrkel;ark_vrf")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qdrvm-crates" CONFIG_PATH "lib/cmake/qdrvm-crates" NO_PREFIX_CORRECTION)

# since config file is moved to share/
vcpkg_replace_string(
  "${CURRENT_PACKAGES_DIR}/share/qdrvm-crates/qdrvm-cratesConfig.cmake"
  "{CMAKE_CURRENT_LIST_DIR}/../../../"
  "{CMAKE_CURRENT_LIST_DIR}/../../"
)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
