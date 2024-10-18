cmake_minimum_required(VERSION 3.5.1)

project(bgt_example)

set(STATIC_BUILD ON CACHE BOOL "" FORCE)


if(NOT TARGET strata_static)
    set(STRATA_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../bin")

    include(${STRATA_BIN_DIR}/cmake/Standalone.cmake)

    add_compile_definitions(${STRATA_PLATFORM_DEFINES})
    include_directories("${STRATA_BIN_DIR}/includes")
    link_directories("${STRATA_LIB_DIR}")
endif()


add_executable(bgt_example
    config.hpp
    bgt_example.cpp
)

if(STATIC_BUILD)
    target_link_libraries(bgt_example strata_static${STRATA_LIB_SUFFIX})
    get_external_link_libraries(strata_static STRATA_LIBRARY_DEPENDENCIES)
    if(STRATA_LIBRARY_DEPENDENCIES)
        target_link_libraries(bgt_example ${STRATA_LIBRARY_DEPENDENCIES})
    endif()
else()
    target_link_libraries(bgt_example strata_shared${STRATA_LIB_SUFFIX})
endif()
