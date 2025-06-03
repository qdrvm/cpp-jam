vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
# Force rebuild to debug bandersnatch_vrfs issue
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO qdrvm/kagome-crates
    REF c4a2c5ea9c7b2fda8623066591593a35dc47b927
    SHA512 1c5ae38aa64ac4dca2c37f950785bfdc588127cf2d2a2386744dee911983c7d3944c3d441d709c7eaaa39e41f6786a2b8f11d86b315805c7d4f443533a8e3fac
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        "-DQDRVM_BIND_CRATES=schnorrkel;ark_vrf"
)

vcpkg_cmake_build()

vcpkg_cmake_install()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

# Use vcpkg_cmake_config_fixup to properly move config files
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/qdrvm-crates PACKAGE_NAME qdrvm-crates)
