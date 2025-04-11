vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/kagome-crates
  REF fa437487127bfdc78f15266bb1fddaf0c913d8cf
  SHA512 c186c8f274aa0ec49fc33ebb4aff3b0cc1dc287d48c1ca5408fc0de38302e5e0d405c83849a580739b10456e2f4f5da7f6441cf24777a4e004276de74d0f4a0b
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
