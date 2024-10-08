/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <universal/data_definitions.h>


/**
 * This interface supports control and configuration of a data receiver,
 * as specified by IDataProperties_t, and an implementation specific settings buffer
 */
class IData
{
public:
    virtual ~IData() = default;

    /**
     * Configures the data interface to continuously receive the described data
     * This can be repeated with different configurations
     *
     * @param index index of data receiver
     * @param *dataProperties pointer to memory structure containing the properties
     * @param *settings pointer to buffer containing the settings
     * @param length size of buffer containing the settings
     */
    virtual void configure(uint8_t index, const IDataProperties_t *dataProperties, const uint8_t *settings, uint16_t settingsSize) = 0;

    template <std::size_t N>
    inline void configure(uint8_t index, const IDataProperties_t *dataProperties, const uint8_t (*settings)[N])
    {
        configure(index, dataProperties, *settings, N);
    }

    template <typename T>
    inline void configure(uint8_t index, const IDataProperties_t *dataProperties, const T *settings)
    {
        configure(index, dataProperties, settings->settings, settings->settingsSize);
    }

    /**
     * Start the data receiving hardware
     * @param index index of data receiver
     */
    virtual void start(uint8_t index = 0) = 0;

    /**
     * Stop the data receiving hardware
     * @param index index of data receiver
     */
    virtual void stop(uint8_t index = 0) = 0;

    /**
     * Returns the internal state of the data interface
     *
     * @param index index of data receiver
     * @return mask containing the status flags
     */
    virtual uint32_t getStatusFlags(uint8_t index = 0) = 0;
};
