
set(PLATFORM_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/BoardAny.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BoardDescriptor.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BoardInstance.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BoardListProtocol.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BoardManager.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/NamedMemory.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Memory.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RegisterGenerator.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeControl.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeData.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeProtocol.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeProtocolI2c.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeProtocolGpio.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeProtocolSpi.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeProtocolMemory.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeProtocolFlash.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeProtocolData.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeWrapperBase.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/VendorCommandsImpl.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/BoardEthernetTcp.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/BoardEthernetUdp.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/BoardDescriptorEthernet.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/BridgeEthernet.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/BridgeEthernetControl.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/BridgeEthernetData.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/BridgeEthernetTcp.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/BridgeEthernetUdp.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/EnumeratorEthernet.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/SocketTcp.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/SocketUdp.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/exception/EAlreadyOpened.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/exception/EBridgeData.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/exception/EConnection.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/exception/EInUse.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/exception/EMemory.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/exception/EProtocol.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/exception/EProtocolFunction.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/exception/EUvcProperty.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/fpga/BridgeFpgaIrpli.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/frames/DebugFrame.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/frames/ErrorFrame.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/frames/Frame.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/frames/FrameBase.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/frames/FrameForwarder.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/frames/FrameListenerCaller.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/frames/FramePool.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/frames/FrameQueue.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/frames/FrameHelper.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IBoard.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IBridge.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IBridgeControl.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IBridgeData.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IVendorCommands.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IEnumerator.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IFrame.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IFrameListener.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IFramePool.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IFrameQueue.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/access/IData.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/access/IGpio.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/access/IMemory.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/access/II2c.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/access/ISpi.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/access/IFlash.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/link/ISerialPort.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/link/ISocket.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/link/IMulticoreDebugger.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/link/IUvcExtension.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/macro/BoardInstanceMacro.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/macro/BridgeMacro.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/serial/BoardSerial.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/serial/BridgeSerial.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/serial/SerialPort.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/templates/boardFactoryFunction.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/templates/enumerateFunction.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/templates/identifyBoardFunction.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/templates/searchBoardFunction.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/uvc/BoardUvc.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/uvc/VendorExtensionList.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/uvc/VendorExtensionCypress.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/uvc/VendorExtensionRealtek.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/uvc/VendorExtensionRealtekFlash.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/uvc/VendorExtensionRealtekI2c.hpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/boards/Board.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/boards/BoardGeneric.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/boards/BoardRemote.hpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/impl/${STRATA_TARGET_PLATFORM}/ethernet/SocketImpl.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/impl/${STRATA_TARGET_PLATFORM}/ethernet/SocketTcpImpl.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/impl/${STRATA_TARGET_PLATFORM}/ethernet/SocketUdpImpl.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/impl/${STRATA_TARGET_PLATFORM}/serial/EnumeratorSerialImpl.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/impl/${STRATA_TARGET_PLATFORM}/serial/SerialPortImpl.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/impl/${STRATA_TARGET_PLATFORM}/serial/SerialPortImplBridge.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/impl/${STRATA_TARGET_PLATFORM}/uvc/BridgeUvcImpl.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/impl/${STRATA_TARGET_PLATFORM}/uvc/EnumeratorUvcImpl.hpp"
    )

set(PLATFORM_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/BoardAny.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BoardDescriptor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BoardInstance.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BoardManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/NamedMemory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Memory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RegisterGenerator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeControl.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeData.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeProtocol.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeProtocolI2c.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeProtocolGpio.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeProtocolSpi.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeProtocolMemory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeProtocolFlash.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeProtocolData.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/BridgeWrapperBase.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bridge/VendorCommandsImpl.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/BridgeEthernet.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/BridgeEthernetControl.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/BridgeEthernetData.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/BridgeEthernetTcp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/BridgeEthernetUdp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/EnumeratorEthernet.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/BoardEthernetTcp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/BoardEthernetUdp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/BoardDescriptorEthernet.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/SocketTcp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ethernet/SocketUdp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/fpga/BridgeFpgaIrpli.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/frames/DebugFrame.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/frames/ErrorFrame.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/frames/Frame.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/frames/FrameBase.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/frames/FrameForwarder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/frames/FramePool.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/frames/FrameQueue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/frames/FrameHelper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/macro/BoardInstanceMacro.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/macro/BridgeMacro.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/serial/SerialPort.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/serial/BridgeSerial.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/serial/BoardSerial.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/uvc/BoardUvc.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/uvc/VendorExtensionCypress.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/uvc/VendorExtensionRealtek.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/uvc/VendorExtensionRealtekFlash.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/uvc/VendorExtensionRealtekI2c.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/boards/Board.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/boards/BoardGeneric.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/boards/BoardRemote.cpp"

    "${CMAKE_CURRENT_SOURCE_DIR}/impl/${STRATA_TARGET_PLATFORM}/serial/SerialPortImpl.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/impl/${STRATA_TARGET_PLATFORM}/serial/EnumeratorSerialImpl.cpp"
    )

if(${STRATA_TARGET_PLATFORM} STREQUAL Windows)
    set(PLATFORM_HEADERS
        ${PLATFORM_HEADERS}
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Windows/serial/SerialPortImplAsync.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Windows/video/EDirectShow.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Windows/video/BridgeDirectShow.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Windows/video/BridgeDirectShowDefinitions.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Windows/video/EnumeratorDirectShow.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Windows/video/EnumeratorDirectShowDefinitions.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Windows/video/Win32Utils.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Windows/video/SampleGrabberInterface.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Windows/video/ComDefines.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Windows/video/VidCap.hpp"
        )

    set(PLATFORM_SOURCES
        ${PLATFORM_SOURCES}
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Windows/ethernet/SocketImpl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Windows/ethernet/SocketTcpImpl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Windows/ethernet/SocketUdpImpl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Windows/serial/SerialPortImplAsync.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Windows/video/EnumeratorDirectShow.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Windows/video/BridgeDirectShow.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Windows/video/Win32Utils.cpp"
        )

    set(PLATFORM_LINK_LIBRARIES
        ws2_32 iphlpapi  # platform/ethernet
        strmiids  # iids
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang)")
        # specify Windows Vista, otherwise MinGW defaults to older Windows versions, disabling some current SDK features
        add_compile_definitions(_WIN32_WINNT=0x0600)

        # For mcd headers:
        # It seems that -Wno-cast-function-type is ignored due to-Wall
        # so -w is used to cancel -Wall
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
    endif()
endif()

if(${STRATA_TARGET_PLATFORM} MATCHES "(Android)|(Linux)")
    set(PLATFORM_HEADERS
        ${PLATFORM_HEADERS}
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Linux/video/EnumeratorV4l2.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Linux/video/BridgeV4l2.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Linux/video/FrameV4l2.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Linux/video/FramePoolV4l2.hpp"
        )

    set(PLATFORM_SOURCES
        ${PLATFORM_SOURCES}
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Linux/video/EnumeratorV4l2.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Linux/video/BridgeV4l2.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Linux/video/FrameV4l2.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/Linux/video/FramePoolV4l2.cpp"
        )
endif()

if(${STRATA_TARGET_PLATFORM} MATCHES "(Android)|(Linux)|(Macos)")
    set(PLATFORM_HEADERS
        ${PLATFORM_HEADERS}
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/unix/ethernet/SocketImpl.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/unix/ethernet/SocketTcpImpl.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/unix/ethernet/SocketUdpImpl.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/unix/serial/SerialPortImplBase.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/unix/serial/EnumeratorSerialImplBase.hpp"
        )

    set(PLATFORM_SOURCES
        ${PLATFORM_SOURCES}
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/unix/ethernet/SocketImpl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/unix/ethernet/SocketTcpImpl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/unix/ethernet/SocketUdpImpl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/unix/serial/SerialPortImplBase.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/impl/unix/serial/EnumeratorSerialImplBase.cpp"
        )
endif()

add_library(platform OBJECT ${PLATFORM_HEADERS} ${PLATFORM_SOURCES})
target_link_libraries(platform PUBLIC ${PLATFORM_LINK_LIBRARIES})

find_package(Threads REQUIRED)
target_link_libraries(platform PUBLIC ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries_helper(platform rapidjson)

target_link_libraries(platform PRIVATE pugixml)

if(STRATA_CONNECTION_LIBUSB)
    set(LIBUSB_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/libusb/BoardLibUsb.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/libusb/BridgeLibUsb.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/libusb/EnumeratorLibUsb.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/libusb/LibUsbHelper.hpp"
        )

    set(LIBUSB_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/libusb/BoardLibUsb.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/libusb/BridgeLibUsb.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/libusb/EnumeratorLibUsb.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/libusb/LibUsbHelper.cpp"
        )

    target_sources(platform PRIVATE ${LIBUSB_HEADERS} ${LIBUSB_SOURCES})
    target_compile_definitions(platform PUBLIC STRATA_CONNECTION_LIBUSB)
    if(TARGET libusb_strata)
        target_link_libraries(platform PUBLIC libusb_strata)
    else()
        get_filename_component(LIBUSB_STRATA_DIR ${libusb_strata} DIRECTORY)
        get_filename_component(LIBUSB_STRATA_NAME ${libusb_strata} NAME)
        target_link_directories(platform PUBLIC ${LIBUSB_STRATA_DIR})
        target_link_libraries(platform PUBLIC ${LIBUSB_STRATA_NAME})
    endif()
endif()

if(STRATA_CONNECTION_MCD)
    set(WIGGLER_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/wiggler/BoardWiggler.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/wiggler/BridgeWiggler.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/wiggler/EnumeratorWiggler.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/wiggler/MulticoreDebugger.hpp"
        )

    set(WIGGLER_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/wiggler/BoardWiggler.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/wiggler/BridgeWiggler.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/wiggler/EnumeratorWiggler.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/wiggler/MulticoreDebugger.cpp"
        )

    target_sources(platform PRIVATE ${WIGGLER_HEADERS} ${WIGGLER_SOURCES})
    target_compile_definitions(platform PUBLIC STRATA_CONNECTION_MCD)
    target_link_libraries_helper(platform mcd_api)
endif()
