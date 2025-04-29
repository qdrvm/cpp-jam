file(GLOB_RECURSE SOURCES *.cpp)

add_library(loaders
    ${SOURCES}
)

target_include_directories(loaders
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(loaders
    qtils::qtils
    soralog::soralog
) 