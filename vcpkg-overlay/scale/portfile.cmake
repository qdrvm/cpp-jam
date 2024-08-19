vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/scale-codec-cpp
  REF ab9fb76632aa8ae98978ce3c54ed88a2fd29fd4e
  SHA512 51e013a7a6e40f30f64a490251c37636b13566660c43a5245c5e5c1c3e4de3f18b81b730cacbaccfc5dd9d64b82ddec1ebcf04a47089a9be2353c82a3f3b0f25
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "scale")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
