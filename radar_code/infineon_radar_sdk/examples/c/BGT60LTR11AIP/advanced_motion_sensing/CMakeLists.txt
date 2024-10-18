include(TargetPlatform)

# detect target platform
detect_target_platform(TARGET_PLATFORM)

if(TARGET_PLATFORM STREQUAL "linux_x64" OR
   TARGET_PLATFORM STREQUAL "linux_raspi" OR
   TARGET_PLATFORM STREQUAL "win32_x64")
   set(MOTION_SENSING_SUPPORTED TRUE)
else()
    set(MOTION_SENSING_SUPPORTED FALSE)
endif()

if(${MOTION_SENSING_SUPPORTED})
    link_directories("${RDK_LIB_DIR}/${TARGET_PLATFORM}")

    add_executable(BGT60LTR11AIP_advanced_motion_sensing advanced_motion_sensing.c)
    target_link_libraries(BGT60LTR11AIP_advanced_motion_sensing sdk_ltr11 argparse)

    if(MINGW OR MSYS OR WIN32)
        if(MINGW OR MSYS)
            # for msys give the exact path to the library to avoid linking again
            # radar_sdk.lib
            target_link_libraries(BGT60LTR11AIP_advanced_motion_sensing "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/sdk_advanced_motion_sensing.dll")
        else()
            target_link_libraries(BGT60LTR11AIP_advanced_motion_sensing "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/sdk_advanced_motion_sensing.lib")
        endif()

        add_custom_command(
            TARGET BGT60LTR11AIP_advanced_motion_sensing
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/sdk_advanced_motion_sensing.dll"
            "$<TARGET_FILE_DIR:BGT60LTR11AIP_advanced_motion_sensing>"
        )
    else()
        link_directories("${RDK_LIB_DIR}/${TARGET_PLATFORM}")
        target_link_libraries(BGT60LTR11AIP_advanced_motion_sensing "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/libsdk_advanced_motion_sensing.so")

        add_custom_command(
            TARGET BGT60LTR11AIP_advanced_motion_sensing
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/libsdk_advanced_motion_sensing.so"
            "$<TARGET_FILE_DIR:BGT60LTR11AIP_advanced_motion_sensing>"
        )
    endif()
else()
	message("-- unknown platform '${TARGET_PLATFORM}' - advanced motion sensing will not be available")
endif()
