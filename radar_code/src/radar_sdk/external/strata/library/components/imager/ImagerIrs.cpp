/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "ImagerIrs.hpp"

#include <common/cpp11/iterator.hpp>
#include <thread>


ImagerIrs::ImagerIrs(II2c *accessI2c, uint16_t devAddr, IGpio *accessGpio, const IPinsIrsConfig_t *pinsConfig) :
    m_registers(accessI2c, devAddr),
    m_pins(accessGpio, pinsConfig),
    m_initialized {false},
    m_started {false}
{
}

ImagerIrs::~ImagerIrs()
{
}

IRegisters<uint16_t> *ImagerIrs::getIRegisters()
{
    return &m_registers;
}

IPinsIrs *ImagerIrs::getIPinsIrs()
{
    return &m_pins;
}

void ImagerIrs::reset()
{
    // check if reset pin works
    //m_initialized = false;

    m_pins.setResetPin(false);
    const std::chrono::microseconds delay(2);
    std::this_thread::sleep_for(delay);
    m_pins.setResetPin(true);
}

void ImagerIrs::configure(const BatchType regs[], uint16_t count)
{
    m_registers.write(regs, count);

    m_initialized = true;
}
