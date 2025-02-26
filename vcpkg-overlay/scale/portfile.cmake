vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/scale-codec-cpp
  REF refs/tags/v2.0.0
  SHA512 80ae69a29c094654acccfacb1f3f9fd37ad40725887536b96920055dfa1efda7e0c847975967c809c676d1123d9b9a0cba8706b1b8ab2f6239b7b842442c2fdc
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
