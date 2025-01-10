vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/scale-codec-cpp
  REF 4d068f40d65597f9636fb55a66eb19f0c483bc3b
  SHA512 b0ce6614bec6ae6271ce0295921279d52cc52cce21d9c4ae9648abd3c369d7be5e9d7deaa6f9bc1894486c4f93b8f14eb3c85c6cad703f70aba163c14d895400
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
