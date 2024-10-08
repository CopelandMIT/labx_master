/****************************************************************************\
* Copyright (C) 2021 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#ifndef DATA_SETTINGS_BGT_RADAR
#define DATA_SETTINGS_BGT_RADAR 1


#define DATA_SETTINGS_BGT_RADAR_SIZE(count, aggregation) (2 * sizeof(uint16_t) * ((count) + (((aggregation) > 1) ? 1 : 0)))
#define DATA_SETTINGS_BGT_RADAR_ENTRY(readoutAddress, readoutCount) \
    (readoutAddress) & 0xFF, (readoutAddress) >> 8, (readoutCount) & 0xFF, (readoutCount) >> 8,
#define DATA_SETTINGS_BGT_RADAR_AGGREGATION(aggregation) \
    ((aggregation) - 1) & 0xFF, ((aggregation) - 1) >> 8, 0, 0


#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#include <vector>
#else
#include <stdint.h>
#endif

struct ReadoutEntry_t
{
    uint16_t address;
    uint16_t count;
};

static inline void DataSettingsBgtRadar_initialize(uint8_t *settings, const uint16_t readouts[][2], uint16_t count, uint16_t aggregation)
{
    for (uint16_t i = 0; i < count; i++)
    {
        const uint16_t readoutAddress = readouts[i][0];
        const uint16_t readoutCount   = readouts[i][1];

        *settings++ = readoutAddress & 0xFF;
        *settings++ = readoutAddress >> 8;
        *settings++ = readoutCount & 0xFF;
        *settings++ = readoutCount >> 8;
    }
    if (aggregation > 1)
    {
        aggregation--;
        *settings++ = aggregation & 0xFF;
        *settings++ = aggregation >> 8;
        *settings++ = 0;
        *settings   = 0;
    }
}

#ifdef __cplusplus

struct DataSettingsBgtRadar_t
{
    DataSettingsBgtRadar_t() :
        settings {nullptr},
        settingsSize {0}
    {}

    DataSettingsBgtRadar_t(const uint16_t readouts[][2], uint16_t count, uint16_t aggregation = 0) :
        DataSettingsBgtRadar_t()
    {
        initialize(readouts, count, aggregation);
    }

    template <size_t N>
    DataSettingsBgtRadar_t(const uint16_t (*readouts)[N][2], uint16_t aggregation = 0) :
        DataSettingsBgtRadar_t(*readouts, N, aggregation)
    {}

    ~DataSettingsBgtRadar_t()
    {
        if (settings)
        {
            delete[] settings;
        }
    }

    inline void allocate(uint16_t newSize)
    {
        if (settings)
        {
            delete[] settings;
            settings = nullptr;
        }

        settingsSize = newSize;

        if (settingsSize)
        {
            settings = new uint8_t[settingsSize];
        }
    }

    inline void initialize(const uint16_t readouts[][2], uint16_t count, uint16_t aggregation = 0)
    {
        allocate(DATA_SETTINGS_BGT_RADAR_SIZE(count, aggregation));
        if (settingsSize == 0)
        {
            return;
        }

        DataSettingsBgtRadar_initialize(settings, readouts, count, aggregation);
    }

    inline void initialize(const std::vector<ReadoutEntry_t> &readoutEntries, uint16_t aggregation = 0)
    {
        allocate(DATA_SETTINGS_BGT_RADAR_SIZE(static_cast<uint16_t>(readoutEntries.size()), aggregation));
        if (settingsSize == 0)
        {
            return;
        }

        uint8_t *ptr = settings;
        for(const auto& readout : readoutEntries)
        {
            const uint16_t readoutAddress = readout.address;
            const uint16_t readoutCount   = readout.count;
            *ptr++ = readoutAddress & 0xFF;
            *ptr++ = readoutAddress >> 8;
            *ptr++ = readoutCount & 0xFF;
            *ptr++ = readoutCount >> 8;
        }
        if (aggregation > 1)
        {
            aggregation--;
            *ptr++ = aggregation & 0xFF;
            *ptr++ = aggregation >> 8;
            *ptr++ = 0;
            *ptr   = 0;
        }
    }

    uint8_t *settings;
    uint16_t settingsSize;
};

#endif

#endif /* DATA_SETTINGS_BGT_RADAR H */
