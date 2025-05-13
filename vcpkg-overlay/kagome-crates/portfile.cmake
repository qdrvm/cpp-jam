vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/kagome-crates
  REF f84cc57eb6c0754753c05623c2f6831197596210
  SHA512 6877b8ba3862a7e5ce7e5627fe1f50f3a7cdc06b1de12246af99eb756b47f114d4c0749d01aaa1ffb64d54645ee389e9535642d3de0b23a8c6d3ed286e1c8340
)
vcpkg_cmake_configure(
  SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
    -DQDRVM_BIND_CRATES=pvm_bindings
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "kagome-crates")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
