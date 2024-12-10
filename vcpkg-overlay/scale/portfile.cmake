vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/scale-codec-cpp
  REF 6b3921740d8c40053a78ed843f7e105dbd978369
  SHA512 0a5464ef5e5785cd3b8af9b97e55ab57d76597c5eceb25ed5678bb95b45388fd6190c178a6670257bfdd2046b6197c330c21b961f4e4ee2e283552e519ebdc1d
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
