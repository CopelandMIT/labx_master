set(BGT60TR13C_COMMON_SOURCES
    common.c
    json.cpp
    time_formatter.c
    util.c
)

set(BGT60TR13C_COMMON_HEADERS
    common.h
    json.h
    json.hpp
    time_formatter.h
    util.h
)

add_library(BGT60TR13C_common STATIC ${BGT60TR13C_COMMON_SOURCES} ${BGT60TR13C_COMMON_HEADERS})
target_include_directories(BGT60TR13C_common PUBLIC .)
target_link_libraries(BGT60TR13C_common PUBLIC sdk_avian sdk_radar)
target_link_libraries(BGT60TR13C_common PRIVATE argparse nlohmann_json)
