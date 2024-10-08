
set(PROTOCOL_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteGasBoyle.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteProtocolAtr22.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteProtocolAvian.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteProtocolLtr11.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteProtocolSmartar.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemotePinsAvian.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemotePinsLtr11.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemotePinsSmartar.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteProcessingRadar.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteRegisters.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteRadarAvian.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteRadarAtr22.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteRadarLtr11.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteRadarSmartar.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteVendorCommands.hpp"
    )

set(PROTOCOL_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteGasBoyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteProtocolAtr22.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteProtocolAvian.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteProtocolLtr11.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteProtocolSmartar.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemotePinsAvian.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemotePinsLtr11.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemotePinsSmartar.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteProcessingRadar.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteRegisters.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteRadarAvian.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteRadarAtr22.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteRadarLtr11.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteRadarSmartar.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RemoteVendorCommands.cpp"
    )

add_library(remote OBJECT ${PROTOCOL_HEADERS} ${PROTOCOL_SOURCES})
