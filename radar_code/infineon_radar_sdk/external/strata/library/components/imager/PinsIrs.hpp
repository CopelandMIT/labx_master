/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <components/interfaces/IPinsIrs.hpp>
#include <platform/interfaces/access/IGpio.hpp>


class PinsIrs :
    public IPinsIrs
{
public:
    PinsIrs(IGpio *accessGpio, const IPinsIrsConfig_t *config);
    virtual ~PinsIrs() = default;

    virtual void setResetPin(bool state) override;
    virtual void setTriggerPin(bool state) override;

private:
    IGpio *m_accessGpio;
    const IPinsIrsConfig_t m_config;
};
