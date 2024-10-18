set(SDK_FMCW_SOURCES
    DeviceFmcwTypes.c
    DeviceFmcw.cpp
    DeviceFmcwBase.cpp
    DeviceFmcwCWrapper.cpp
    MetricsFmcw.cpp
    avian/DeviceFmcwAvian.cpp
    )

set(SDK_FMCW_HEADERS
    DeviceFmcw.h
    DeviceFmcw.hpp
    DeviceFmcwTypes.h
    DeviceFmcwBase.hpp
    MetricsFmcw.h
    avian/DeviceFmcwAvian.hpp
    avian/DeviceFmcwAvianConfig.h
)

add_library(sdk_fmcw SHARED ${SDK_FMCW_SOURCES} ${SDK_FMCW_HEADERS})
target_link_libraries(sdk_fmcw PRIVATE lib_avian)
target_link_libraries(sdk_fmcw PUBLIC sdk_base sdk_radar_device_common)
