
if(WIN32)
    add_definitions(-DNOMINMAX) # prevent min/max macros from being defined in Windows SDK header
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if(STRATA_BUILD_WARNINGS_AS_ERRORS)
        add_compile_options("/WX")  # treat all warnings as errors
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4800")  # suppress stupid bool conversion MSVC compiler warning
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")  # suppress misleading deprecated warning for unchecked iterators
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250")  # suppress inheritance via dominance warning for virtual inheritance

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w34263")  # warn about member function hiding virtual base class function
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w34266")  # warn about derived class not overriding all virtual base class functions
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if(STRATA_BUILD_WARNINGS_AS_ERRORS)
        add_compile_options("-Werror")  # treat all warnings as errors
    endif()

    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-override")  # warn about missing override specifier
    endif()
endif()
