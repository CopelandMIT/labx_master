
strata_include(Compiler)

set(STRATA_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/platform/impl/${STRATA_TARGET_PLATFORM}"
    CACHE INTERNAL ""
    )

add_definitions(-DSTRATA_EXPORT) # this will tell the windows compiler to generate the DLL export file for the linker
add_definitions(${STRATA_PLATFORM_DEFINES})
include_directories(${STRATA_INCLUDE_DIRS})


set(STRATA_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/Definitions.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Library.hpp"
    )

set(STRATA_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Library.cpp"
    )

add_library(strata_library OBJECT ${STRATA_HEADERS} ${STRATA_SOURCES})

add_library(strata_boards OBJECT
    BoardList.hpp
    BoardList.cpp
    )

#This object defines the version function. Link it or define the version function by yourself
add_library(strata_version OBJECT
    Version.cpp
    )
target_compile_definitions(strata_version PUBLIC ${STRATA_VERSION_DEFINES})

add_subdirectory(common)
add_subdirectory(components)
add_subdirectory(modules)
add_subdirectory(platform)
add_subdirectory(remote)
add_subdirectory(universal)


set(STRATA_LIBRARY_OBJECTS
    strata_library
    common
    components
    modules
    platform
    remote
    universal
    CACHE INTERNAL ""
)

add_library(strata_static STATIC Dummy.cpp)
target_link_libraries_helper(strata_static
                             strata_boards
                             strata_version
                             ${STRATA_LIBRARY_OBJECTS}
                             )

if(STRATA_SHARED_LIBRARY)
    add_library(strata_shared SHARED Dummy.cpp)
    target_link_libraries_helper(strata_shared
                                 strata_boards
                                 strata_version
                                 ${STRATA_LIBRARY_OBJECTS}
                                 )
    target_compile_definitions(strata_shared INTERFACE "STRATA_IMPORT")
endif()


#file(GLOB_RECURSE PUBLIC_LIBRARY_HEADERS */interfaces/*.hpp)
set(PUBLIC_LIBRARY_INTERFACE_HEADERS
    "Definitions.hpp"
    "Library.hpp"
    "common/cpp11/memory.hpp"
    "common/cpp11/iterator.hpp"
    "components/interfaces/IComponent.hpp"
    "components/interfaces/IProcessingRadar.hpp"
    "components/interfaces/IRadarAvian.hpp"
    "components/interfaces/IPinsAvian.hpp"
    "components/interfaces/IProtocolAvian.hpp"
    "components/interfaces/IRegisters.hpp"
    "modules/interfaces/IModule.hpp"
    "platform/interfaces/IBoard.hpp"
    "platform/interfaces/IBridge.hpp"
    "platform/interfaces/IBridgeControl.hpp"
    "platform/interfaces/IBridgeData.hpp"
    "platform/interfaces/IEnumerator.hpp"
    "platform/interfaces/IFrame.hpp"
    "platform/interfaces/IFrameListener.hpp"
    "platform/interfaces/IVendorCommands.hpp"
    "platform/interfaces/access/IData.hpp"
    "platform/interfaces/access/IGpio.hpp"
    "platform/interfaces/access/IMemory.hpp"
    "platform/interfaces/access/II2c.hpp"
    "platform/interfaces/access/ISpi.hpp"
    "platform/interfaces/access/IFlash.hpp"
#    "platform/interfaces/link/ISerialPort.hpp"
#    "platform/interfaces/link/ISocket.hpp"
#    "platform/interfaces/link/ISocketUdp.hpp"
#    "platform/interfaces/link/ISocketTcp.hpp"
#    "platform/interfaces/link/IMulticoreDebugger.hpp"
#    "platform/interfaces/link/IUvcExtension.hpp"
    "platform/BoardDescriptor.hpp"
    "platform/BoardInstance.hpp"
    "platform/BoardManager.hpp"
    "platform/NamedMemory.hpp"
    "universal/data_definitions.h"
    "universal/error_definitions.h"
    "universal/gpio_definitions.h"
    "universal/i2c_definitions.h"
    "universal/spi_definitions.h"
    "universal/components/subinterfaces.h"
    "universal/components/power_supply.h"
    "universal/components/processing.h"
    "universal/components/radar.h"
    "universal/modules/subinterfaces.h"
    "universal/types/IfxRfe_Types.h"
    "universal/types/IfxRsp_Types.h"
    "universal/types/AurixPlatformDataSettings.h"
    "universal/types/DataSettingsBgtRadar.h"
    )

# clean include files from output directory on install
install(CODE "file(REMOVE_RECURSE ${STRATA_BINARY_DIR}/includes)")

foreach(file ${PUBLIC_LIBRARY_INTERFACE_HEADERS})
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${file} DESTINATION ${STRATA_BINARY_DIR}/includes/${dir})
endforeach()


set(INSTALL_BINARY_DIR ${STRATA_BINARY_DIR}/${STRATA_TARGET_PLATFORM}/x${STRATA_BITNESS})
get_external_link_libraries(strata_static STRATA_LIBRARY_DEPENDENCIES)
install(CODE "file(WRITE \"${INSTALL_BINARY_DIR}/deps.txt\" \"${STRATA_LIBRARY_DEPENDENCIES}\")")

install(TARGETS strata_static DESTINATION ${INSTALL_BINARY_DIR})

if(TARGET strata_shared)
    install(TARGETS strata_shared DESTINATION ${INSTALL_BINARY_DIR})
endif()

if(TARGET libusb_strata)
    install(TARGETS libusb_strata DESTINATION ${INSTALL_BINARY_DIR})
endif()


install(FILES "${STRATA_DIR}/cmake/Platform.cmake" DESTINATION ${STRATA_BINARY_DIR}/cmake)
install(FILES "${STRATA_DIR}/cmake/Standalone.cmake" DESTINATION ${STRATA_BINARY_DIR}/cmake)
