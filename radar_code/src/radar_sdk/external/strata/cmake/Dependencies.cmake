
macro(add_recursive_dependencies target output type)
    foreach(dep ${ARGN})
        if (NOT ${dep} IN_LIST ${output})
            if (TARGET ${dep})
                get_target_property(dep_type ${dep} TYPE)
                if (dep_type STREQUAL ${type})
                    list(APPEND ${output} ${dep})
                    get_target_property(libs ${dep} LINK_LIBRARIES)
                    add_recursive_dependencies(${target} ${output} ${type} ${libs})
                endif()
            endif()
        endif()
    endforeach()
endmacro()

function(get_recursive_dependencies target output type)
    set(OUTPUT_LIST "")
    add_recursive_dependencies(${target} OUTPUT_LIST ${type} ${ARGN})
    list(REMOVE_DUPLICATES OUTPUT_LIST)
    set(${output} ${OUTPUT_LIST} PARENT_SCOPE)
endfunction()


function(strata_copy_dependencies target)
    if(TARGET libusb_strata)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:libusb_strata> $<TARGET_FILE_DIR:${target}>
            )
    endif()
endfunction()

function(strata_set_test_path_dependencies name)
    if(TARGET libusb_strata)
        # this does not work for python tests, which rely on the file in the same directory as the module
        list(APPEND env_path $<TARGET_FILE_DIR:libusb_strata>)
    endif()

    foreach(arg IN LISTS ARGN)
        if(TARGET ${arg})
            get_target_property(target_type ${arg} TYPE)
            if(target_type STREQUAL SHARED_LIBRARY)
                # if the test links to a shared library, it needs to know where to find it
                # therefore, the execution environment will be adapted with a path to the dependency
                list(APPEND env_path $<TARGET_FILE_DIR:${arg}>)
            endif()
        else()
            list(APPEND env_path ${arg})
        endif()
    endforeach()

    list(APPEND env_path $ENV{PATH})

    if(UNIX)
        string(REPLACE ";" ":" env_path "${env_path}")
    elseif(WIN32)
        # semicolons within an environment variable need to be escaped,
        # since the list of variables itself is separated by semicolons
        string(REPLACE ";" "\\;" env_path "${env_path}")
    endif()

    set_tests_properties(${name} PROPERTIES ENVIRONMENT "PATH=${env_path}")
    set_target_properties(${name} PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=${env_path}")
endfunction()
