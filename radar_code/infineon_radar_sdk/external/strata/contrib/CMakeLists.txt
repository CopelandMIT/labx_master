
unset(CMAKE_CXX_CLANG_TIDY) # do not run static analysis on contrib code (in case it was enabled before)

file(GLOB CONTRIB_FILES *.cmake)
add_custom_target(_contrib SOURCES ${CONTRIB_FILES})


if(STRATA_BUILD_TESTS)
    include(catch2.cmake)
    include(gtest.cmake)
endif()

if(STRATA_CONNECTION_LIBUSB)
    include(libusb.cmake)
endif()

if(STRATA_CONNECTION_MCD)
    add_subdirectory(mcd_api)
endif()

include(rapidjson.cmake)
include(pugixml.cmake)

if(STRATA_BUILD_SAMPLES)
# removed unused qwt for now
#    include(qwt.cmake OPTIONAL)
endif()
