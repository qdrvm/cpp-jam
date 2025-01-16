#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

set(BUILD_VERSION_CPP "${CMAKE_CURRENT_BINARY_DIR}/build_version.cpp")
add_custom_command(
    OUTPUT ${BUILD_VERSION_CPP}
    COMMAND echo "#include <string>" > ${BUILD_VERSION_CPP}
    COMMAND echo "namespace jam {" >> ${BUILD_VERSION_CPP}
    COMMAND echo "  const std::string &buildVersion() {" >> ${BUILD_VERSION_CPP}
    COMMAND printf "    static const std::string buildVersion(\"" >> ${BUILD_VERSION_CPP}
    COMMAND ${PROJECT_SOURCE_DIR}/get_version.sh >> ${BUILD_VERSION_CPP}
    COMMAND echo "\");" >> ${BUILD_VERSION_CPP}
    COMMAND echo "    return buildVersion;" >> ${BUILD_VERSION_CPP}
    COMMAND echo "  }" >> ${BUILD_VERSION_CPP}
    COMMAND echo "}" >> ${BUILD_VERSION_CPP}
    COMMENT "Generate build_version.cpp"
    VERBATIM
)
add_library(build_version
    ${CMAKE_CURRENT_BINARY_DIR}/build_version.cpp
)

add_library(app_configurator SHARED configurator.cpp)
target_link_libraries(app_configurator
    yaml-cpp::yaml-cpp
    Boost::program_options
    build_version
)

add_library(app_configuration SHARED configuration.cpp)
#target_link_libraries(app_configuration
#  yaml-cpp::yaml-cpp)

add_library(app_state_manager SHARED impl/state_manager_impl.cpp)
target_link_libraries(app_state_manager
    logger
)

add_library(application SHARED impl/application_impl.cpp)
target_link_libraries(application
    qtils::qtils
    soralog::soralog
    app_configuration
)
