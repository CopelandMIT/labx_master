
set(STRATA_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE STRING "Directory for Strata files")
message(STATUS "Strata directory: ${STRATA_DIR}")

set(STRATA_BINARY_DIR "${STRATA_DIR}/bin")


file(GLOB STRATA_CMAKE_FILES cmake/*.cmake)
if(STRATA_BUILD_WRAPPERS)
    set(STRATA_CMAKE_FILES ${STRATA_CMAKE_FILES}
        wrappers/matlab.cmake
        wrappers/pybindPythonMultiVersion.cmake
        wrappers/pybindPythonVersion.cmake
        wrappers/python.cmake
        )
endif()

add_custom_target(Strata SOURCES
    ${STRATA_CMAKE_FILES}
    Strata.cmake
    VERSION
    )


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_DEBUG_POSTFIX -d)
set(CMAKE_POSITION_INDEPENDENT_CODE True)

if (CMAKE_BUILD_TYPE MATCHES [Dd][Ee][Bb][Uu][Gg])
    set(STRATA_DEBUG_BUILD True)
endif()

if(NOT STRATA_DEBUG_BUILD)
    add_definitions(-DNDEBUG)
endif()

if(STRATA_BUILD_TESTS)
    enable_testing()
    include(CTest)

    # clear global test lists for test functions to add them
    set(STRATA_INTEGRATION_TESTS CACHE INTERNAL "")
    set(STRATA_UNIT_TESTS CACHE INTERNAL "")
endif()


include(cmake/Dependencies.cmake)
include(cmake/Helper.cmake)
include(cmake/Install.cmake)
include(cmake/Platform.cmake)
include(cmake/Version.cmake)

file(READ "${STRATA_DIR}/VERSION" VERSION)
set_version(STRATA ${VERSION})
message(STATUS "Strata version: ${STRATA_VERSION_MAJOR}.${STRATA_VERSION_MINOR}.${STRATA_VERSION_PATCH}.${STRATA_VERSION_BUILD}")
message(STATUS "Strata target architecture/platform: ${STRATA_TARGET_ARCHITECTURE} ${STRATA_BITNESS}bit ${STRATA_TARGET_PLATFORM}")


# Include Custom.cmake if it is present
if(EXISTS "${STRATA_DIR}/Custom.cmake")
    message(STATUS "Including custom cmake file : ${STRATA_DIR}/Custom.cmake")
    include("${STRATA_DIR}/Custom.cmake")
endif()


macro(strata_include cmake_file)
    include("${STRATA_DIR}/cmake/${cmake_file}.cmake")
endmacro()
