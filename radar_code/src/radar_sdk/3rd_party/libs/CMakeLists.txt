
add_subdirectories(EXCLUDE_FROM_ALL)


# The following section disables specific compiler warnings in 3rd party code.
# We don't want to change it, and also not want it to pullute our log.

if(CMAKE_C_COMPILER_ID MATCHES "MSVC")
    if (TARGET zip)
        target_compile_options(zip PRIVATE
            /wd4244
            /wd4267
            )
    endif()

    if (TARGET CMSISDSP)
        target_compile_options(CMSISDSP PRIVATE
            /wd4244
            )
    endif()
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    if (TARGET bossa)
        target_compile_options(bossa PRIVATE
            -Wno-cast-function-type
        )
    endif()

    if (TARGET muFFT)
        target_compile_options(muFFT PRIVATE
            -std=gnu99
        )
    endif()

    if(TARGET zip)
        target_compile_options(zip PRIVATE
            -Wno-stringop-truncation
        )
    endif()
endif()

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    if (CMAKE_CXX_SIMULATE_ID MATCHES "MSVC")
        set(MUFFT_COMPILE_DEFINES
            -Wno-undef
            -Wno-declaration-after-statement
            -Wno-shadow
        )

        if(CMAKE_C_COMPILER_VERSION GREATER_EQUAL 16)
            set(MUFFT_COMPILE_DEFINES
                ${MUFFT_COMPILE_DEFINES}
                -Wno-unsafe-buffer-usage
            )
        endif()

        if (TARGET muFFT)
            target_compile_options(muFFT PRIVATE
                ${MUFFT_COMPILE_DEFINES}
                -Wno-implicit-int-float-conversion
                -Wno-sign-conversion
                -Wno-nonportable-system-include-path
                -Wno-unused-macros
            )
        endif()

        if (TARGET muFFT-avx)
            target_compile_options(muFFT-avx PRIVATE
                ${MUFFT_COMPILE_DEFINES}
                -Wno-reserved-macro-identifier
                -Wno-cast-align
            )
        endif()

        if (TARGET muFFT-sse)
            target_compile_options(muFFT-sse PRIVATE
                ${MUFFT_COMPILE_DEFINES}
                -Wno-reserved-macro-identifier
            )
        endif()

        if (TARGET muFFT-sse3)
            target_compile_options(muFFT-sse3 PRIVATE
                ${MUFFT_COMPILE_DEFINES}
                -Wno-reserved-macro-identifier
            )
        endif()
    endif()

    if (TARGET zip)
        target_compile_options(zip PRIVATE
            -Wno-strict-prototypes
            )
    endif()
endif()
