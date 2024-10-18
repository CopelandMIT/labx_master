/****************************************************************************\
* Copyright (C) 2021 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#ifndef AURIX_PLATFORM_DATA_SETTINGS
#define AURIX_PLATFORM_DATA_SETTINGS 1


#define AURIX_PLATFORM_DATA_SETTINGS_FLAGS_LSB_FIRST   (1u << 0)  ///< specifies whether LSB is transmitted first (otherwise MSB first)
#define AURIX_PLATFORM_DATA_SETTINGS_FLAGS_CRC_ENABLED (1u << 1)  ///< enable transmission and check of CRC


#define AURIX_PLATFORM_DATA_SETTINGS_SIZE (4)

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>

typedef uint8_t AurixPlatformDataSettings_t[AURIX_PLATFORM_DATA_SETTINGS_SIZE];
#endif


static inline void AurixPlatformDataSettings_initialize(uint8_t (*settings)[AURIX_PLATFORM_DATA_SETTINGS_SIZE], uint8_t flags)
{
        (*settings)[0] = flags;
        (*settings)[1] = 0;  // placeholder for backwards compatibility
        (*settings)[2] = 0;  // placeholder for backwards compatibility
        (*settings)[3] = 0;  // placeholder for backwards compatibility
}


#ifdef __cplusplus

struct AurixPlatformDataSettings_t
{
    AurixPlatformDataSettings_t(uint8_t flags)
    {
        initialize(flags);
    }

    inline void initialize(uint8_t flags)
    {
        AurixPlatformDataSettings_initialize(&settings, flags);
    }

    uint8_t settings[AURIX_PLATFORM_DATA_SETTINGS_SIZE];

    static constexpr uint16_t settingsSize = AURIX_PLATFORM_DATA_SETTINGS_SIZE;
};

#endif

#endif /* AURIX_PLATFORM_DATA_SETTINGS H */
