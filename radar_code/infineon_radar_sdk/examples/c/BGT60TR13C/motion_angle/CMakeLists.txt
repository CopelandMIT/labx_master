include(TargetPlatform)

# detect target platform
detect_target_platform(TARGET_PLATFORM)

if(TARGET_PLATFORM STREQUAL "linux_x64" OR
   TARGET_PLATFORM STREQUAL "linux_raspi" OR
   TARGET_PLATFORM STREQUAL "win32_x64")
   set(MOTIONANGLE_SUPPORTED TRUE)
else()
    set(MOTIONANGLE_SUPPORTED FALSE)
endif()

if(${MOTIONANGLE_SUPPORTED})
    link_directories("${RDK_LIB_DIR}/${TARGET_PLATFORM}")

    add_executable(BGT60TR13C_motion_angle motion_angle.c)
    target_link_libraries(BGT60TR13C_motion_angle BGT60TR13C_common sdk_avian)

    if(MINGW OR MSYS OR WIN32)
        if(MINGW OR MSYS)
            # for msys give the exact path to the library to avoid linking again
            # radar_sdk.lib
            target_link_libraries(BGT60TR13C_motion_angle "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/sdk_motionangle.dll")
        else()
            target_link_libraries(BGT60TR13C_motion_angle "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/sdk_motionangle.lib")
        endif()

        add_custom_command(
            TARGET BGT60TR13C_motion_angle
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/sdk_motionangle.dll"
            "$<TARGET_FILE_DIR:BGT60TR13C_motion_angle>"
        )
    else()
        link_directories("${RDK_LIB_DIR}/${TARGET_PLATFORM}")
        target_link_libraries(BGT60TR13C_motion_angle "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/libsdk_motionangle.so")

        add_custom_command(
            TARGET BGT60TR13C_motion_angle
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/libsdk_motionangle.so"
            "$<TARGET_FILE_DIR:BGT60TR13C_motion_angle>"
        )
    endif()
else()
	message("-- unknown platform '${TARGET_PLATFORM}' - motion-angle will not be available")
endif()
