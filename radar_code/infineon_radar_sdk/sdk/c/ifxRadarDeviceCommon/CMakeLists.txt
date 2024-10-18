set(SDK_RADAR_DEVICE_COMMON_SOURCES
    RadarDeviceCommon.cpp
)

set(SDK_RADAR_DEVICE_COMMON_HEADERS
    RadarDeviceCommon.h
    internal/RadarDeviceCommon.hpp
)

add_library(sdk_radar_device_common SHARED ${SDK_RADAR_DEVICE_COMMON_SOURCES} ${SDK_RADAR_DEVICE_COMMON_HEADERS})
target_link_libraries(sdk_radar_device_common PUBLIC sdk_base)
target_link_libraries(sdk_radar_device_common PRIVATE lib_avian)
