set(SDK_CW_SOURCES
    DeviceCwTypes.c
    DeviceCw.cpp
    DeviceCwBase.cpp
    avian/DeviceCwAvian.cpp
)

set(SDK_CW_HEADERS
    DeviceCwTypes.h
    DeviceCw.h
    DeviceCw.hpp
    DeviceCwBase.hpp
    avian/DeviceCwAvian.hpp
)

add_library(sdk_cw SHARED ${SDK_CW_SOURCES} ${SDK_CW_HEADERS})
target_link_libraries(sdk_cw PRIVATE lib_avian)
target_link_libraries(sdk_cw PUBLIC sdk_base sdk_radar_device_common)
