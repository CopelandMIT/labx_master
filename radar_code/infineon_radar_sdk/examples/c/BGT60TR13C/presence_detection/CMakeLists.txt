include(TargetPlatform)

# detect target platform
detect_target_platform(TARGET_PLATFORM)

if(TARGET_PLATFORM STREQUAL "linux_x64" OR
   TARGET_PLATFORM STREQUAL "linux_raspi" OR
   TARGET_PLATFORM STREQUAL "win32_x64")
   set(PRESENCE_DETECTION_SUPPORTED TRUE)
else()
    set(PRESENCE_DETECTION_SUPPORTED FALSE)
endif()

if(${PRESENCE_DETECTION_SUPPORTED})
    link_directories("${RDK_LIB_DIR}/${TARGET_PLATFORM}")

    add_executable(BGT60TR13C_presence_detection presence_detection.c)
    target_link_libraries(BGT60TR13C_presence_detection sdk_avian BGT60TR13C_common)

    if(MINGW OR MSYS OR WIN32)
        if(MINGW OR MSYS)
            # for msys give the exact path to the library to avoid linking again
            # radar_sdk.lib
            target_link_libraries(BGT60TR13C_presence_detection "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/sdk_presence_sensing.dll")
        else()
            target_link_libraries(BGT60TR13C_presence_detection "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/sdk_presence_sensing.lib")
        endif()

        add_custom_command(
            TARGET BGT60TR13C_presence_detection
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/sdk_presence_sensing.dll"
            "$<TARGET_FILE_DIR:BGT60TR13C_presence_detection>"
        )
    else()
        link_directories("${RDK_LIB_DIR}/${TARGET_PLATFORM}")
        target_link_libraries(BGT60TR13C_presence_detection "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/sdk_presence_sensing.so")

        add_custom_command(
            TARGET BGT60TR13C_presence_detection
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/sdk_presence_sensing.so"
            "$<TARGET_FILE_DIR:BGT60TR13C_presence_detection>"
        )
    endif()
else()
	message("-- unknown platform '${TARGET_PLATFORM}' - sdk_presence_sensing will not be available")
endif()
