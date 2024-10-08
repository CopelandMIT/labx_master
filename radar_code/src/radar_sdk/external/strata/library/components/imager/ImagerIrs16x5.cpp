/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "ImagerIrs16x5.hpp"

#include <common/cpp11/iterator.hpp>
#include <thread>


namespace
{
    //  const uint16_t TRIG    = 0x9833;
    const uint16_t EN      = 0xB400;
    const uint16_t CTRL    = 0xB401;
    const uint16_t LPFSMEN = 0x9000;
}


ImagerIrs16x5::ImagerIrs16x5(II2c *accessI2c, uint16_t devAddr, IGpio *accessGpio, const IPinsIrsConfig_t *pinsConfig) :
    ImagerIrs(accessI2c, devAddr, accessGpio, pinsConfig)
{
}

ImagerIrs16x5::~ImagerIrs16x5()
{
    ImagerIrs16x5::stopImager();
}

void ImagerIrs16x5::setupSystemPll(const IRegisters<uint16_t>::BatchType regs[], uint16_t count)
{
    m_registers.write(regs, count);

    const auto delay = std::chrono::milliseconds(2);
    std::this_thread::sleep_for(delay);
    m_registers.write(CTRL, 0x001);
    m_registers.write(EN, 0x001);
    std::this_thread::sleep_for(delay);
}

void ImagerIrs16x5::startImager()
{
    if (m_started)
    {
        return;
    }

    m_registers.write(LPFSMEN, 0x0005);
    m_started = true;
}

void ImagerIrs16x5::stopImager()
{
    if (!m_started)
    {
        return;
    }

    m_registers.write(LPFSMEN, 0x0001);
    m_started = false;
}
