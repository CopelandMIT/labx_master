

set(CMAKE_EXPORT_COMPILE_COMMANDS On)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES Off) # clang-tidy does not expand response files, which leads to errors


if(CLANG_TIDY_PATH)
    set(CLANG_TIDY_EXE ${CLANG_TIDY_PATH}clang-tidy)
else()
    # We have to use find_program at configuration time to use the correct path.
    # At runtime there could be an environment from Visual Studio, where a different clang-format
    # version is found, which will not work. For this reason the complete path is set here.
    find_program(CLANG_TIDY_EXE "clang-tidy")
endif()


set(CLANG_TIDY_OPTIONS
    --quiet
    )

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
         # explicitly specify target, so the system headers are handled correctly
         set(CLANG_TIDY_OPTIONS ${CLANG_TIDY_OPTIONS} --extra-arg-before=--target=x86_64-w64-mingw32)
    endif()
endif()

if(STRATA_BUILD_ANALYZE_CODE)
    message(STATUS " -> Running static analysis when compiling (clang-tidy)")
    message(WARNING "    This will fail if the build system uses response files, since clang-tidy is missing support for them\n"
                    "    (see https://reviews.llvm.org/D34440)"
        )

    set(CMAKE_CXX_CLANG_TIDY
        ${CLANG_TIDY_EXE}
        ${CLANG_TIDY_OPTIONS}
        )
endif()


strata_include(Filesystem)

function(strata_add_analyze_targets NAME FILE_EXTENSIONS)
    strata_list_files(FILES "${FILE_EXTENSIONS}" ${ARGN})

    add_custom_target(${NAME}_all
        SOURCES .clang-tidy
        )
    add_custom_command(TARGET ${NAME}_all
        COMMAND ${CLANG_TIDY_EXE} ARGS
        ${CLANG_TIDY_OPTIONS}
        -p="${PROJECT_BINARY_DIR}"
        ${FILES}
        )

    add_custom_target(${NAME}_incremental
        SOURCES .clang-tidy
        )
    foreach(file ${FILES})
        add_custom_command(TARGET ${NAME}_incremental
            COMMAND ${CLANG_TIDY_EXE} ARGS
            ${CLANG_TIDY_OPTIONS}
            -p="${PROJECT_BINARY_DIR}"
            ${file}
            )
    endforeach()
endfunction()
