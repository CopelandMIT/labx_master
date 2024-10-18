
function(strata_add_python_test name wrapper_target type)
    add_custom_target(${name}_dep
        DEPENDS ${wrapper_target}
        )

    add_custom_target(${name}
        COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG> -R ${name} --output-on-failure
        SOURCES ${ARGN}
        DEPENDS ${wrapper_target}
        )

    add_test(NAME ${name}
        COMMAND ${Python3_EXECUTABLE} -m unittest discover -s "${CMAKE_CURRENT_SOURCE_DIR}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
    set_tests_properties(${name} PROPERTIES LABELS ${type})

    set(python_path $<TARGET_FILE_DIR:${wrapper_target}>)
    list(APPEND python_path $ENV{PYTHONPATH})
    if(UNIX)
        string(REPLACE ";" ":" python_path "${python_path}")
    elseif(WIN32)
        # semicolons within an environment variable need to be escaped,
        # since the list of variables itself is separated by semicolons
        string(REPLACE ";" "\\;" python_path "${python_path}")
    endif()
    set_property(TEST ${name} APPEND PROPERTY ENVIRONMENT "PYTHONPATH=${python_path}")

    # add test to global list for test handling
    set(STRATA_${type}_TESTS ${STRATA_${type}_TESTS} ${name}_dep CACHE INTERNAL "")
endfunction()
