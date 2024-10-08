/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <components/imager/PinsIrs.hpp>
#include <components/imager/RegistersIrs.hpp>
#include <components/interfaces/IImager.hpp>


class ImagerIrs :
    public IImager
{
public:
    ImagerIrs(II2c *accessI2c, uint16_t devAddr, IGpio *accessGpio, const IPinsIrsConfig_t *pinsConfig);
    ~ImagerIrs();

    IRegisters<uint16_t> *getIRegisters() override;
    IPinsIrs *getIPinsIrs() override;

    void reset() override;

    using IImager::configure;
    void configure(const BatchType regs[], uint16_t count) override;


protected:
    RegistersIrs m_registers;
    PinsIrs m_pins;

    bool m_initialized;
    bool m_started;
};
