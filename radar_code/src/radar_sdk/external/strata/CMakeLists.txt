option(STRATA_PACKAGES_AUTO_DETECT "find and build additional packages automatically" ON)
option(STRATA_SHARED_LIBRARY "build shared library" ON)

option(STRATA_BUILD_SAMPLES "build sample applications" OFF)
option(STRATA_BUILD_TESTS "build integration and unit tests" OFF)
option(STRATA_BUILD_TOOLS "build tool applications" OFF)
option(STRATA_BUILD_WRAPPERS "try to build wrappers for Matlab, Python, etc." ON)

option(STRATA_BUILD_ANALYZE_CODE "run static code analyzer during build" OFF)
option(STRATA_BUILD_WARNINGS_AS_ERRORS "make build fail if compiler emits any warning" OFF)
option(STRATA_TARGET_ANALYZE "add targets to run code analyzer" ON)
option(STRATA_TARGET_FORMAT "add targets to run code formatter" ON)
if(STRATA_BUILD_TESTS)
    option(STRATA_BUILD_RUN_UNIT_TESTS "run unit tests after a build" OFF)
endif()

option(STRATA_CONNECTION_MCD "build with Multicore Debugger connection support" OFF)
option(STRATA_CONNECTION_LIBUSB "build with LibUsb connection support" ON)

# if STRATA_MULTIPLE_PYTHON_WRAPPER_VERSIONS is enabled pybind11MultiVersion and the conan python package will be used for the build
option(STRATA_MULTIPLE_PYTHON_WRAPPER_VERSIONS "build the python wrapper for multiple python versions at once" OFF)

#set(STRATA_PACKAGES "") # STRATA_PACKAGES_AUTO_DETECT has to be disabled, since it overwrites this list


cmake_minimum_required(VERSION 3.5.1)
project(Strata)
include(Strata.cmake)
include(${CMAKE_SOURCE_DIR}/../local_settings.cmake OPTIONAL)

# Conan can be used to build Strata, yet if it is used it has to be called externally.
# Then it is detected here and can be used.
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup(TARGETS NO_OUTPUT_DIRS)
    MESSAGE(STATUS "Conan package ${CONAN_PACKAGE_NAME}, version ${CONAN_PACKAGE_VERSION}, build type ${CMAKE_BUILD_TYPE}")
endif()

if(STRATA_TARGET_ANALYZE)
    strata_include(Analyze)
    strata_add_analyze_targets(analyze "*.cpp" library)
endif()

if(STRATA_TARGET_FORMAT)
    strata_include(Format)
    strata_add_format_targets(format_strata "*.?pp;*.h" library samples tests tools wrappers packages)
endif()


add_subdirectory_optional(contrib)

add_subdirectory(library)

if(STRATA_BUILD_WRAPPERS)
    include(wrappers/matlab.cmake)
    include(wrappers/python.cmake)

    add_subdirectory(wrappers)
endif()
if(STRATA_BUILD_TESTS)
    set(STRATA_INTEGRATION_TEST_OUTPUT_DIR "${PROJECT_BINARY_DIR}/tests/output/integration_tests")
    set(STRATA_UNIT_TEST_OUTPUT_DIR "${PROJECT_BINARY_DIR}/tests/output/unit_tests")

    add_subdirectory(tests)
endif()

if(STRATA_BUILD_SAMPLES)
    add_subdirectory(samples)
endif()
if(STRATA_BUILD_TOOLS)
    add_subdirectory(tools)
endif()


add_subdirectory_optional(packages)


if(STRATA_BUILD_TESTS)
    if(STRATA_BUILD_RUN_UNIT_TESTS)
        set(UNIT_TESTS_ALL ALL)
    endif()

    add_custom_target(unit_tests ${UNIT_TESTS_ALL} SOURCES tests/dummy)
    add_dependencies(unit_tests ${STRATA_UNIT_TESTS})
    add_custom_command(TARGET unit_tests
        POST_BUILD
        COMMAND ${CMAKE_CTEST_COMMAND}
        ARGS -C $<CONFIG> -L "UNIT" --output-on-failure
        )
endif()
