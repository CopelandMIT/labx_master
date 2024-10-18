
# on the following platforms, use system libusb
if(${STRATA_TARGET_PLATFORM} STREQUAL Linux)
    find_library(libusb_strata NAMES usb usb-1.0)
    if(libusb_strata)
        return()
    endif()
    message(WARNING "LibUsb could not be found, so it will be built here.\nHowever, it is highly recommended to use a globally installed version on linux-based systems!")
endif()


set(LIBUSB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libusb")
set(LIBUSB_SOURCE_DIR "${LIBUSB_DIR}/libusb-1.0")
set(LIBUSB_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libusb_config")

if(NOT EXISTS ${LIBUSB_DIR})
    return()
endif()


file(GLOB LIBUSB_HEADERS
    "${LIBUSB_SOURCE_DIR}/*.h")
file(GLOB LIBUSB_SOURCES
    "${LIBUSB_SOURCE_DIR}/*.c")

# libusb needs to be a shared library due to license restrictionsi
add_library(libusb_strata SHARED ${LIBUSB_HEADERS} ${LIBUSB_SOURCES})


set(LIBUSB_POSIX_HEADERS
    "${LIBUSB_SOURCE_DIR}/os/events_posix.h"
    "${LIBUSB_SOURCE_DIR}/os/threads_posix.h"
    )
set(LIBUSB_POSIX_SOURCES
    "${LIBUSB_SOURCE_DIR}/os/events_posix.c"
    "${LIBUSB_SOURCE_DIR}/os/threads_posix.c"
    )

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
    target_compile_options(libusb_strata PRIVATE
                           -Wno-unused-but-set-variable # suppress warning about unused variable that is checked by assert
                           )
endif()

if(${STRATA_TARGET_PLATFORM} STREQUAL Windows)
    # libusb doesn't specify windows exports explicitly, so we need to explicitly export everything
    set_target_properties(libusb_strata PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS True)

    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(LIBUSB_PLATFORM_CONFIG_DIR "${LIBUSB_CONFIG_DIR}/msvc")
    else()
        set(LIBUSB_PLATFORM_CONFIG_DIR "${LIBUSB_CONFIG_DIR}/mingw")

        target_compile_options(libusb_strata PRIVATE
                               -Wno-implicit-function-declaration # suppress warning about implicit function declaration due to missing include of windows.h
                               -Wno-cast-function-type            # suppress warning about function pointer cast
                               )
    endif()

    set(LIBUSB_PLATFORM_HEADERS
        "${LIBUSB_SOURCE_DIR}/os/windows_common.h"
        "${LIBUSB_SOURCE_DIR}/os/windows_usbdk.h"
        "${LIBUSB_SOURCE_DIR}/os/windows_winusb.h"
        )
    set(LIBUSB_PLATFORM_SOURCES
        "${LIBUSB_SOURCE_DIR}/os/windows_common.c"
        "${LIBUSB_SOURCE_DIR}/os/windows_usbdk.c"
        "${LIBUSB_SOURCE_DIR}/os/windows_winusb.c"
        )

    list(APPEND LIBUSB_PLATFORM_HEADERS
        "${LIBUSB_SOURCE_DIR}/os/events_windows.h"
        "${LIBUSB_SOURCE_DIR}/os/threads_windows.h"
        )
    list(APPEND LIBUSB_PLATFORM_SOURCES
        "${LIBUSB_SOURCE_DIR}/os/events_windows.c"
        "${LIBUSB_SOURCE_DIR}/os/threads_windows.c"
        )
elseif(${STRATA_TARGET_PLATFORM} STREQUAL Macos)
    set(LIBUSB_PLATFORM_CONFIG_DIR "${LIBUSB_CONFIG_DIR}/Xcode")

    set(LIBUSB_PLATFORM_HEADERS
        "${LIBUSB_SOURCE_DIR}/os/darwin_usb.h"
        )
    set(LIBUSB_PLATFORM_SOURCES
        "${LIBUSB_SOURCE_DIR}/os/darwin_usb.c"
        )

    list(APPEND LIBUSB_PLATFORM_HEADERS ${LIBUSB_POSIX_HEADERS})
    list(APPEND LIBUSB_PLATFORM_SOURCES ${LIBUSB_POSIX_SOURCES})

    find_library(framework_corefoundation CoreFoundation REQUIRED)
    find_library(framework_driverkit DriverKit REQUIRED)
    find_library(framework_iokit IOKit REQUIRED)
    find_library(framework_security Security REQUIRED)
    target_link_libraries(libusb_strata PRIVATE
                          ${framework_corefoundation}
                          ${framework_driverkit}
                          ${framework_iokit}
                          ${framework_security}
                          )

elseif(${STRATA_TARGET_PLATFORM} STREQUAL Android)
    target_compile_options(libusb_strata PRIVATE -fvisibility=hidden)
    target_link_libraries(libusb_strata PUBLIC log)

    set(LIBUSB_PLATFORM_CONFIG_DIR "${LIBUSB_CONFIG_DIR}/android")

    set(LIBUSB_PLATFORM_HEADERS
        "${LIBUSB_SOURCE_DIR}/os/linux_usbfs.h"
        )
    set(LIBUSB_PLATFORM_SOURCES
        "${LIBUSB_SOURCE_DIR}/os/linux_usbfs.c"
        )

    list(APPEND LIBUSB_PLATFORM_HEADERS ${LIBUSB_POSIX_HEADERS})
    list(APPEND LIBUSB_PLATFORM_SOURCES ${LIBUSB_POSIX_SOURCES})
else() # Linux
    set(LIBUSB_PLATFORM_CONFIG_DIR "${LIBUSB_CONFIG_DIR}/linux")

    set(LIBUSB_PLATFORM_HEADERS
        "${LIBUSB_SOURCE_DIR}/os/linux_usbfs.h"
        )
    set(LIBUSB_PLATFORM_SOURCES
        "${LIBUSB_SOURCE_DIR}/os/linux_netlink.c"
        "${LIBUSB_SOURCE_DIR}/os/linux_usbfs.c"
        "${LIBUSB_SOURCE_DIR}/os/linux_udev.c"
        )

    find_library(lib_udev udev REQUIRED)
    target_link_libraries(libusb_strata PRIVATE ${lib_udev})

    list(APPEND LIBUSB_PLATFORM_HEADERS ${LIBUSB_POSIX_HEADERS})
    list(APPEND LIBUSB_PLATFORM_SOURCES ${LIBUSB_POSIX_SOURCES})
endif()


target_sources(libusb_strata PRIVATE
    ${LIBUSB_PLATFORM_HEADERS}
    ${LIBUSB_PLATFORM_SOURCES}
    "${LIBUSB_PLATFORM_CONFIG_DIR}/config.h"
    )
target_include_directories(libusb_strata
    PUBLIC "${LIBUSB_DIR}"
    PRIVATE "${LIBUSB_SOURCE_DIR}" "${LIBUSB_PLATFORM_CONFIG_DIR}"
    )
