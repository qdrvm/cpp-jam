vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/kagome-crates
  REF 79271951cc3a90ceddd7a917f81b42ae43912c5a
  SHA512 e8b7fe834c3e02b9ac70b87af08b13cd0aee98aac7e856dd5931981dbbdde2e1e388e9e3e479b9a2b155a62a487a13c0407333929a57517856d730bfa894dec4
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}" OPTIONS "-DQDRVM_BIND_CRATES=bandersnatch_vrfs;schnorrkel;ark_ec_vrfs")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qdrvm-crates" CONFIG_PATH "lib/cmake/qdrvm-crates" NO_PREFIX_CORRECTION)

# since config file is moved to share/
vcpkg_replace_string(
  "${CURRENT_PACKAGES_DIR}/share/qdrvm-crates/qdrvm-cratesConfig.cmake"
  "{CMAKE_CURRENT_LIST_DIR}/../../../"
  "{CMAKE_CURRENT_LIST_DIR}/../../"
)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
