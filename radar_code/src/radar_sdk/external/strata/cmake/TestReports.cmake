
if(ALLURE_PATH)
    set(ALLURE_BINARY ${ALLURE_PATH}allure)
else()
    # We have to use find_program at configuration time to use the correct path.
    # At runtime there could be an environment from Visual Studio, where a different clang-format
    # version is found, which will not work. For this reason the complete path is set here.
    find_program(ALLURE_BINARY "allure")
endif()

set(ALLURE_COMMAND ${ALLURE_BINARY} generate --clean)


function(add_integration_test_report_target name output label)
    foreach(test ${STRATA_INTEGRATION_TESTS})
        #todo: use LABELS for matching
        #get_test_property(${test} LABELS labels)
        #if(${labels} MATCHES ${label})
        if(${test} MATCHES ${label})
            list(APPEND LABELED_TESTS ${test})
        endif()
    endforeach()
    add_custom_target(${name}
        SOURCES ${STRATA_DIR}/tests/dummy
        DEPENDS ${LABELED_TESTS}
        )
    add_custom_command(TARGET ${name}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${STRATA_INTEGRATION_TEST_OUTPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${STRATA_INTEGRATION_TEST_OUTPUT_DIR}
        )
    add_custom_command(TARGET ${name}
        POST_BUILD
        COMMAND ${CMAKE_CTEST_COMMAND}
        ARGS -C $<CONFIG> -R \"${label}\" -T Test
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        )
    add_custom_command(TARGET ${name}
        POST_BUILD
        COMMAND ${ALLURE_COMMAND} ${STRATA_INTEGRATION_TEST_OUTPUT_DIR}
        ARGS -o ${output}
        )
endfunction()


function(add_unit_test_report_target name output)
    string(REPLACE ";" "$$|^" REGEX_LIST "(^${STRATA_UNIT_TESTS}$$)")
    add_custom_target(${name}
        SOURCES ${STRATA_DIR}/tests/dummy
        DEPENDS ${STRATA_UNIT_TESTS}
        )
    add_custom_command(TARGET ${name}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${STRATA_UNIT_TEST_OUTPUT_DIR}
        )
    add_custom_command(TARGET ${name}
        POST_BUILD
        COMMAND ${CMAKE_CTEST_COMMAND}
        ARGS -C $<CONFIG> -R \"${REGEX_LIST}\" -L "UNIT" -T Test
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        )
    add_custom_command(TARGET ${name}
        POST_BUILD
        COMMAND ${ALLURE_COMMAND} ${STRATA_UNIT_TEST_OUTPUT_DIR}
        ARGS -o ${output}
        )
endfunction()
