vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/kagome-crates
  REF 9011440d56d51bb33e7c4c6a47e85314178ce87f
  SHA512 f0b1b0e1ed6e20d149d5639e14c231a46f3cdaaa6bfa320bfd64e88c4ed0496fa4246c3135e0032647c05c2aaac02804b565fbeae34b1625771114c8b263920d
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "kagome-crates")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
