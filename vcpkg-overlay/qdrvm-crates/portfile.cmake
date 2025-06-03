vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/kagome-crates
  REF 9e4fba63ed5ce685ff01a24bb70aa5f62b8715b2
  SHA512 04b0b68dd8c4b333db3eb05209795d04a4d213c62e919fefd5e210fb1874554bc0595307946bde9a824bade44c2fd603a1cfb52199e11fdb7367b4ce0abcb8ef
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}" OPTIONS
  "-DQDRVM_BIND_CRATES=schnorrkel;ark_vrf"
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qdrvm-crates")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
