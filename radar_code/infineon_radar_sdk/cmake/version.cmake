# get project's version from VERSION file
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" SDK_VERSION)
string(STRIP "${SDK_VERSION}" SDK_VERSION)
if (SDK_VERSION)
    message(STATUS "SDK version: ${SDK_VERSION}")
else()
    message(WARNING "VERSION file not found in the project's root")
    set(SDK_VERSION "0.0.0")
endif()

# get hash of the current git commit
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE SDK_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(SDK_VERSION_FULL "${SDK_VERSION}+${SDK_COMMIT_HASH}")
