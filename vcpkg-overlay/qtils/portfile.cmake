vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
#  REF refs/tags/v0.1.0
  REF 163287d1e521788ebccc74bfcf8d1199cb25643b
  SHA512 bd4d1f319dc2c810a2f6204d93f63c67f4b0139ddd9dedb9e374f8cb964a1ad7eb46f93f667b3b248abf8949b1f577e86c53a98aae04af926373c98c2c7582ee
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
