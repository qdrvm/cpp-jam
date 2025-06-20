#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

set(BUILD_VERSION_CPP "${CMAKE_BINARY_DIR}/generated/app/build_version.cpp")
set(GET_VERSION_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/get_version.sh")
execute_process(
    COMMAND ${GET_VERSION_SCRIPT}
    OUTPUT_VARIABLE VERSION_SCRIPT_OUTPUT
)
file(
    WRITE ${BUILD_VERSION_CPP}
    "// Auto-generated file\n"
    "#include <string>\n"
    "namespace morum {\n"
    "  const std::string &buildVersion() {\n"
    "    static const std::string buildVersion(\"${VERSION_SCRIPT_OUTPUT}\");\n"
    "    return buildVersion;\n"
    "  }\n"
    "}"
)
add_library(build_version
    ${BUILD_VERSION_CPP}
)

add_library(app_configuration SHARED configuration.cpp)
target_link_libraries(app_configuration
    Boost::boost
    fmt::fmt
)

add_library(app_configurator SHARED configurator.cpp)
target_link_libraries(app_configurator
    app_configuration
    yaml-cpp::yaml-cpp
    Boost::program_options
    build_version
)

add_library(app_state_manager SHARED impl/state_manager_impl.cpp)
target_link_libraries(app_state_manager
    qtils::qtils
    logger
)

add_library(application SHARED impl/application_impl.cpp)
target_link_libraries(application
    qtils::qtils
    app_configuration
    metrics
)
