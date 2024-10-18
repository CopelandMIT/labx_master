set(SDK_MIMOSE_SOURCES
    DeviceMimose.cpp
    DeviceMimoseImpl.cpp
    DeviceMimoseBase.cpp
    DeviceMimoseDummy.cpp
    DeviceMimoseRegisterConfigurator.cpp
)

set(SDK_MIMOSE_HEADERS
    DeviceMimose.h
    DeviceMimoseTypes.h
    DeviceMimose.h
    DeviceMimoseImpl.hpp
    DeviceMimoseBase.hpp
    DeviceMimoseDummy.hpp
    DeviceMimoseRegisterConfigurator.hpp
    DeviceMimoseConstants.hpp
    registers_BGT24ATR22.hpp
)

add_library(sdk_mimose SHARED ${SDK_MIMOSE_SOURCES} ${SDK_MIMOSE_HEADERS})
target_link_libraries(sdk_mimose PUBLIC sdk_base sdk_radar_device_common)
