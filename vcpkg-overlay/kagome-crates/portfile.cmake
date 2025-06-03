vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/kagome-crates
  REF c4a2c5ea9c7b2fda8623066591593a35dc47b927
  SHA512 1c5ae38aa64ac4dca2c37f950785bfdc588127cf2d2a2386744dee911983c7d3944c3d441d709c7eaaa39e41f6786a2b8f11d86b315805c7d4f443533a8e3fac
)
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DQDRVM_BIND_CRATES=schnorrkel;bandersnatch_vrfs
)
vcpkg_cmake_install()

# Rename the share directory to match the package name qdrvm-crates
file(RENAME 
    "${CURRENT_PACKAGES_DIR}/share/kagome-crates" 
    "${CURRENT_PACKAGES_DIR}/share/qdrvm-crates"
)

# The config file is actually created with this name
file(RENAME 
    "${CURRENT_PACKAGES_DIR}/share/qdrvm-crates/kagome-crates-config.cmake" 
    "${CURRENT_PACKAGES_DIR}/share/qdrvm-crates/qdrvm-cratesConfig.cmake"
)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/lib/cmake")
