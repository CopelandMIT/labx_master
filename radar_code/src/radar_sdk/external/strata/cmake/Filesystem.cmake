
set(FILESYSTEM_PLATFORM_REGEX ${STRATA_TARGET_PLATFORM})
if(${STRATA_TARGET_PLATFORM} MATCHES "(Linux)|(Macos)")
    set(FILESYSTEM_PLATFORM_REGEX ${FILESYSTEM_PLATFORM_REGEX}|unix)
endif()


function(strata_list_dirs RESULT)
    set(RESULT_LIST "")
    foreach(dir ${ARGN})
        file(GLOB_RECURSE SUB_DIRS
            RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            LIST_DIRECTORIES True
            ${dir}/.
            )
        list(APPEND RESULT_LIST ${dir})
        foreach(subdir ${SUB_DIRS})
#            if(NOT ${subdir} MATCHES "^(?(?=.*impl\\/).*impl\\/(${PLATFORM_REGEX})|.*)$")
            if(${subdir} MATCHES ".*impl\\/.*" AND NOT ${subdir} MATCHES "(${FILESYSTEM_PLATFORM_REGEX})")
                continue()
            endif()
            list(APPEND RESULT_LIST ${subdir})
        endforeach()
    endforeach()

    set(${RESULT} ${RESULT_LIST} PARENT_SCOPE)
endfunction()

function(strata_list_files RESULT FILE_EXTENSIONS)
    strata_list_dirs(SEARCH_DIRS ${ARGN})

    set(RESULT_LIST "")
    foreach(dir ${SEARCH_DIRS})
        foreach(ext ${FILE_EXTENSIONS})
            file(GLOB FILES ${dir}/${ext})
            if(FILES)
                list(APPEND RESULT_LIST ${FILES})
            endif()
        endforeach()
    endforeach()

    set(${RESULT} ${RESULT_LIST} PARENT_SCOPE)
endfunction()
