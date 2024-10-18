
set(RES_FILES
    "${STRATA_DIR}/resource/version.h"
    "${STRATA_DIR}/resource/version.rc"
    )

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # also explicitly build manifest
    list(APPEND RES_FILES
        "${STRATA_DIR}/resource/manifest.rc"
          )
endif()


function(set_resource_info target)
    set(prefix VER)
    set(options OPTIONAL)
    set(oneValueArgs VERSION FILETYPE)
    set(multiValueArgs COMPANYNAME ORIGINALFILENAME INTERNALNAME PRODUCTNAME FILEDESCRIPTION)
    cmake_parse_arguments("${prefix}" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (${CMAKE_RC_COMPILER} MATCHES ".*windres.*")
        # windres cannot handle quoted strings with spaces, so replace them by escape sequence
        string(REPLACE " " "\\x20" VER_COMPANYNAME ${VER_COMPANYNAME})
        string(REPLACE " " "\\x20" VER_ORIGINALFILENAME ${VER_ORIGINALFILENAME})
        string(REPLACE " " "\\x20" VER_INTERNALNAME ${VER_INTERNALNAME})
        string(REPLACE " " "\\x20" VER_PRODUCTNAME ${VER_PRODUCTNAME})
        string(REPLACE " " "\\x20" VER_FILEDESCRIPTION ${VER_FILEDESCRIPTION})
    endif()

    string(REPLACE "." "," VER_VERSION_NUMBER ${VER_VERSION})

    get_target_property(type ${target} TYPE)

    if(NOT VER_FILETYPE)
        if (${type} STREQUAL STATIC_LIBRARY)
            set(VER_FILETYPE VFT_STATIC_LIB)
        elseif(${type} STREQUAL EXECUTABLE)
            set(VER_FILETYPE VFT_APP)
        else()
            set(VER_FILETYPE VFT_DLL)
        endif()
    endif()

    if(NOT (type STREQUAL MODULE_LIBRARY AND CMAKE_SYSTEM_NAME MATCHES "Darwin"))
        set_target_properties(${target}
            PROPERTIES
            VERSION ${VER_VERSION}
            )
    endif()

    target_sources(${target} PRIVATE ${RES_FILES})
    target_compile_definitions(${target} PRIVATE
        VER_FILEVERSION=${VER_VERSION_NUMBER}
        VER_FILEVERSION_STR=${VER_VERSION}
        VER_PRODUCTVERSION=${VER_VERSION_NUMBER}
        VER_PRODUCTVERSION_STR=${VER_VERSION}
        VER_FILETYPE=${VER_FILETYPE}
        VER_COMPANYNAME="${VER_COMPANYNAME}"
        VER_ORIGINALFILENAME="${VER_ORIGINALFILENAME}"
        VER_INTERNALNAME="${VER_INTERNALNAME}"
        VER_PRODUCTNAME="${VER_PRODUCTNAME}"
        VER_FILEDESCRIPTION="${VER_FILEDESCRIPTION}"
        )
endfunction()
