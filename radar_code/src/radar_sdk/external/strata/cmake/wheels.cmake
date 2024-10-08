find_package(Python3 COMPONENTS Interpreter REQUIRED)

strata_include(Dependencies)


set(PY_PROJECT_FILE "pyproject.toml")

# RDK wheels include binaries built for a specific platform,
# therefore, built wheels shall be limited only to that platform.
# This is achieved through the platform tag, which is passed to
# the wheel build engine (e.g. bdist_wheel)
# To get the list of compatible tags, for a certain system, search for `Compatible tags`
# in the output of the following command:
# python -m pip debug -v
# Additional information on compatibility tags is available here:
# https://packaging.python.org/en/latest/specifications/platform-compatibility-tags/
# Now, let's create a default platform tag (in case it was not provided)
if(NOT WHEEL_PLATFORM_TAG)
    execute_process(COMMAND ${Python3_EXECUTABLE} -c "import distutils.util; print(distutils.util.get_platform())" OUTPUT_VARIABLE WHEEL_PLATFORM_TAG OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()


function(add_wheel)
    set(prefix WHEEL)
    set(options ALL)
    set(oneValueArgs NAME DEPENDENCIES_DIR)
    set(multiValueArgs DEPENDENCIES)
    cmake_parse_arguments(${prefix} "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(WHEEL_PACKAGE_NAME ${WHEEL_NAME})

    set(target wheel-${WHEEL_NAME})
    if(WHEEL_ALL)
        set(WHEEL_ALL ALL)
    else()
        set(WHEEL_ALL)
    endif()

    file(GLOB_RECURSE SRC_FILES src/*.py)

    add_custom_target(${target} ${WHEEL_ALL}
        SOURCES ${PY_PROJECT_FILE} ${SRC_FILES}
        )
    
    if(WHEEL_DEPENDENCIES_DIR)
        set(WHEEL_DEPENDENCIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/${WHEEL_PACKAGE_NAME}/${WHEEL_DEPENDENCIES_DIR}")
        add_custom_command(TARGET ${target} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${WHEEL_DEPENDENCIES_DIR}"
            )
        add_custom_command(TARGET ${target}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${WHEEL_DEPENDENCIES_DIR}"
            )
    else()
        set(WHEEL_DEPENDENCIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/${WHEEL_PACKAGE_NAME}")
    endif()

    add_custom_command(TARGET ${target} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${WHEEL_DEPENDENCIES_DIR}"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_SOURCE_DIR}/build"
        )

    if(CMAKE_LIBRARY_OUTPUT_DIRECTORY)
        set(TARGET_BINARY_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    else()
        set(TARGET_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    if (CMAKE_CONFIGURATION_TYPES)
        # for multi configuration generators we need to manually handle the binary directory suffix
        set(TARGET_BINARY_DIR ${TARGET_BINARY_DIR}/$<CONFIG>)
        add_custom_command(TARGET ${target} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${TARGET_BINARY_DIR}
            )
    endif()

    add_dependencies(${target} ${WHEEL_DEPEDENCIES})
    get_recursive_dependencies(${target} deps SHARED_LIBRARY ${WHEEL_DEPEDENCIES})

    foreach(dep ${deps})
        add_custom_command(TARGET ${target}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${dep}> "${WHEEL_DEPENDENCIES_DIR}"
            )
    endforeach()


    add_custom_command(TARGET ${target}
#        COMMAND ${Python3_EXECUTABLE} -m pip wheel --no-deps . -w "${TARGET_BINARY_DIR}"
        COMMAND ${Python3_EXECUTABLE} -m build -w -o "${TARGET_BINARY_DIR}"
            -C=--build-option=--plat-name=${WHEEL_PLATFORM_TAG}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        )

endfunction()
