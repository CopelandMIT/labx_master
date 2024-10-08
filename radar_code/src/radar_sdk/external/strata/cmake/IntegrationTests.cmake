
cmake_minimum_required( VERSION 3.0 )

set_property( DIRECTORY PROPERTY CXX_STANDARD 11 )

# Add desired warnings
if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    add_compile_options(-Wall -Wextra -Wunreachable-code)
endif()
# Clang specific warning go here
if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    # Actually keep these
    add_compile_options(-Wweak-vtables -Wexit-time-destructors -Wglobal-constructors -Wmissing-noreturn)
endif()
if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    # remove /Wx from options to avoid override warning
    string(REGEX REPLACE "/W[0-9]" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    add_compile_options(/W4 /w44265)
endif()


if(NOT TARGET integration_test_output_dir)
    add_custom_target(integration_test_output_dir
        COMMAND ${CMAKE_COMMAND} -E make_directory ${STRATA_INTEGRATION_TEST_OUTPUT_DIR}
        )
endif()

function(strata_add_integration_test name)
    add_executable(${name} ${name}.cpp ${ARGN} $<TARGET_OBJECTS:CatchMain>)
    add_dependencies(${name} integration_test_output_dir)
    target_include_directories(${name} PUBLIC "${STRATA_DIR}/tests")
    target_link_libraries(${name} ${STRATA_TEST_LINK_LIBRARIES})
    target_link_libraries_helper(${name} catch2)

    add_test(NAME ${name} COMMAND ${name} -r junit -o ${STRATA_INTEGRATION_TEST_OUTPUT_DIR}/${name}.xml)
    set_tests_properties(${name} PROPERTIES LABELS "INTEGRATION")
    strata_set_test_path_dependencies(${name})

    # add integration test to global list for test handling
    set(STRATA_INTEGRATION_TESTS ${STRATA_INTEGRATION_TESTS} ${name} CACHE INTERNAL "")
endfunction()
