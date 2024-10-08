set(SDK_LTR11_SOURCES
    DeviceLtr11.cpp
    DeviceLtr11Impl.cpp
    DeviceLtr11Base.cpp
    DeviceLtr11Dummy.cpp
    DeviceLtr11RegisterConfigurator.cpp
)

set(SDK_LTR11_HEADERS
    DeviceLtr11.h
    DeviceLtr11Impl.hpp
    DeviceLtr11Types.h
    DeviceLtr11Base.hpp
    DeviceLtr11Dummy.hpp
    DeviceLtr11RegisterConfigurator.hpp
)

add_library(sdk_ltr11 SHARED ${SDK_LTR11_SOURCES} ${SDK_LTR11_HEADERS})
target_link_libraries(sdk_ltr11 PUBLIC sdk_base sdk_radar_device_common)
