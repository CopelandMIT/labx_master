if(MSVC)
    # Visual Studio
    # /EHsc: synchronous exception handling
    set(CMAKE_CXX_FLAGS_INIT                "/W3 /EHsc")
    set(CMAKE_CXX_FLAGS_DEBUG_INIT          "/D_DEBUG /MDd /Zi /Ob0 /Od /RTC1")
    set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT     "/MD /O1 /Ob1 /D NDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE_INIT        "/MD /O2 /Ob2 /D NDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "/MD /Zi /O2 /Ob1 /D NDEBUG")
else()
    # gcc and clang
    set(CMAKE_CXX_FLAGS_INIT                "${CMAKE_CXX_FLAGS_INIT} -Wall -Wextra -Wno-unused-parameter")
    set(CMAKE_CXX_FLAGS_DEBUG_INIT          "${CMAKE_CXX_FLAGS_DEBUG_INIT} -g")
    set(CMAKE_CXX_FLAGS_ASAN_INIT           "-g -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined")
    set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT     "-Os -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE_INIT        "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-O2 -g")
endif()
