/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "PinsIrs.hpp"

namespace
{
    const IPinsIrsConfig_t defaultConfig = {
        GPIO_NAME_RESET,
        GPIO_NAME_NONE,
    };
}

PinsIrs::PinsIrs(IGpio *accessGpio, const IPinsIrsConfig_t *config) :
    m_accessGpio {accessGpio},
    m_config(config ? *config : defaultConfig)
{
    m_accessGpio->configurePin(m_config.gpioReset, GPIO_MODE_OUTPUT_OPEN_DRAIN_PULL_UP | GPIO_FLAG_OUTPUT_INITIAL_HIGH);
    m_accessGpio->configurePin(m_config.gpioTrigger, GPIO_MODE_OUTPUT_PUSH_PULL);
}

void PinsIrs::setResetPin(bool state)
{
    m_accessGpio->setPin(m_config.gpioReset, state);
}

void PinsIrs::setTriggerPin(bool state)
{
    m_accessGpio->setPin(m_config.gpioTrigger, state);
}
