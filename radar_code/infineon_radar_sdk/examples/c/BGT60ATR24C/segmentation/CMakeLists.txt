include(TargetPlatform)

# detect target platform
detect_target_platform(TARGET_PLATFORM)

if(TARGET_PLATFORM STREQUAL "linux_x64" OR
   TARGET_PLATFORM STREQUAL "linux_raspi" OR
   TARGET_PLATFORM STREQUAL "win32_x64")
   set(SEGMENTATION_SUPPORTED TRUE)
else()
    set(SEGMENTATION_SUPPORTED FALSE)
endif()

if(${SEGMENTATION_SUPPORTED})
    link_directories("${RDK_LIB_DIR}/${TARGET_PLATFORM}")

    add_executable(BGT60ATR24C_segmentation segmentation.c)
    target_link_libraries(BGT60ATR24C_segmentation sdk_avian BGT60ATR24C_common)

    if(MINGW OR MSYS OR WIN32)
        if(MINGW OR MSYS)
            # for msys give the exact path to the library to avoid linking again
            # radar_sdk.lib
            target_link_libraries(BGT60ATR24C_segmentation "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/sdk_radar_segmentation.dll")
        else()
            target_link_libraries(BGT60ATR24C_segmentation "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/sdk_radar_segmentation.lib")
        endif()

        add_custom_command(
            TARGET BGT60ATR24C_segmentation
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/sdk_radar_segmentation.dll"
            "$<TARGET_FILE_DIR:BGT60ATR24C_segmentation>"
        )
    else()
        link_directories("${RDK_LIB_DIR}/${TARGET_PLATFORM}")
        target_link_libraries(BGT60ATR24C_segmentation "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/libsdk_radar_segmentation.so")

        add_custom_command(
            TARGET BGT60ATR24C_segmentation
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/libs/${TARGET_PLATFORM}/libsdk_radar_segmentation.so"
            "$<TARGET_FILE_DIR:BGT60ATR24C_segmentation>"
        )
    endif()
else()
	message("-- unknown platform '${TARGET_PLATFORM}' - segmentation will not be available")
endif()
