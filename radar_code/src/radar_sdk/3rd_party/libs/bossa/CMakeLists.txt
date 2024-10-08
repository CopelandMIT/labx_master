set(BOSSA_SOURCES
    Applet.cpp
    D2xNvmFlash.cpp
    D5xNvmFlash.cpp
    Device.cpp
    EefcFlash.cpp
    EfcFlash.cpp
    Flash.cpp
    Flasher.cpp
    LinuxPortFactory.cpp
    OSXPortFactory.cpp
    PosixSerialPort.cpp
    Samba.cpp
    WinPortFactory.cpp
    WinSerialPort.cpp
    WordCopyApplet.cpp
    WordCopyArm.cpp)

set(BOSSA_HEADERS
    Applet.h
    D2xNvmFlash.h
    D5xNvmFlash.h
    Device.h
    EefcFlash.h
    EfcFlash.h
    FileError.h
    Flash.h
    Flasher.h
    LinuxPortFactory.h
    OSXPortFactory.cpp
    OSXPortFactory.h
    PortFactory.h
    PosixSerialPort.h
    Samba.cpp
    Samba.h
    SerialPort.h
    Shell.h
    WinPortFactory.h
    WinSerialPort.h
    WordCopyApplet.h
    WordCopyArm.h)

# bossa
add_library(bossa STATIC ${BOSSA_SOURCES} ${BOSSA_HEADERS})
target_compile_definitions(bossa PUBLIC _CRT_SECURE_NO_WARNINGS)
target_include_directories(bossa PUBLIC .)