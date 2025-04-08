vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/kagome-crates
  REF a052aa3dc7d5233bc8f70ac39a9380f20bafe3a6
  SHA512 5a8884668a80a3a9f1251f3dd48a8c7a252f9d893a1fe4c353cea0cc4343c7b25462d10573e7b9227584770cede6a6a2f31dbbdf5483616e0d24b83671db8269
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}" OPTIONS "-DQDRVM_BIND_CRATES=bandersnatch_vrfs;schnorrkel")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qdrvm-crates" CONFIG_PATH "lib/cmake/qdrvm-crates" NO_PREFIX_CORRECTION)

# since config file is moved to share/
vcpkg_replace_string(
  "${CURRENT_PACKAGES_DIR}/share/qdrvm-crates/qdrvm-cratesConfig.cmake"
  "{CMAKE_CURRENT_LIST_DIR}/../../../"
  "{CMAKE_CURRENT_LIST_DIR}/../../"
)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
