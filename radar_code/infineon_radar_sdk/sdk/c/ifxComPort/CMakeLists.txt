set(SDK_COMPORT_SOURCES
    COMPort_Unix.c
    COMPort_Windows.c
    ComPort.cpp
)

set(SDK_COMPORT_HEADERS
    COMPort.h
    internal/ComPort.hpp
)

add_library(sdk_comport_obj OBJECT ${SDK_COMPORT_SOURCES} ${SDK_COMPORT_HEADERS})
target_include_directories(sdk_comport_obj PUBLIC ..)

if(WIN32 OR MINGW OR MSYS)
    target_link_libraries(sdk_comport_obj PRIVATE winmm setupapi)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(sdk_comport_obj PUBLIC "-framework CoreFoundation -framework IOKit")
endif()
