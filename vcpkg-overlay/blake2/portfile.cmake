vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO BLAKE2/BLAKE2
  REF ed1974ea83433eba7b2d95c5dcd9ac33cb847913
  SHA512 5c4bad10659ef2d3eec2b4ba1a13b45055910e28d4c843a1e39cba9999bfcd8232a58e7aa44d603b434cc87255fdaed7aa3de5fedd57738c69775b9569bd271d
)

file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION ${SOURCE_PATH})
vcpkg_cmake_configure(SOURCE_PATH ${SOURCE_PATH})
vcpkg_cmake_install()
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/COPYING")
