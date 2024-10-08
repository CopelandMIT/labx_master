set(SDK_AVIAN_SOURCES
    DeviceConfig.cpp
    DeviceControl.cpp
    Metrics.cpp
)

set(SDK_AVIAN_HEADERS
    Avian.h
    DeviceConfig.h
    DeviceControl.h
    Metrics.cpp
    Metrics.h
)

add_library(sdk_avian SHARED ${SDK_AVIAN_SOURCES} ${SDK_AVIAN_HEADERS})
target_link_libraries(sdk_avian PRIVATE lib_avian)
target_link_libraries(sdk_avian PUBLIC sdk_fmcw)
