if(MSVC)
    # Visual Studio

    # treat warnings as errors
    add_compile_options("/WX")
    add_link_options("/WX")
else()
    # gcc and clang

    # treat warnings as errors
    add_compile_options("-Werror")

    if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_link_options("-Wl,--fatal-warnings")
    endif()

    # For clang on MinGW use lld as linker. lld is faster when it comes to
    # linking C++ object files with a large number of symbols compared to ld.
    # (This is actually a bug/limitation on MinGW.) Also, using sanitizers
    # works on MinGW only with clang in combination with lld.
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND MINGW)
        add_compile_options("-fuse-ld=lld")

        # lld by default uses multiple threads. As there are bugs in the
        # winpthread that lld uses, using lld with multiple threads might
        # result in lld hanging indefinetly. To avoid this, we disable
        # multi-threading support for lld.
        add_link_options("LINKER:/threads:no")
    endif()

    if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11)
        add_compile_options("-Wno-psabi")
    endif()

endif()
