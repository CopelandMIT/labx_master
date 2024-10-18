/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "ImagerIrs.hpp"

class ImagerIrs16x5 :
    public ImagerIrs
{
public:
    ImagerIrs16x5(II2c *accessI2c, uint16_t devAddr, IGpio *accessGpio, const IPinsIrsConfig_t *pinsConfig = nullptr);
    ~ImagerIrs16x5();

    void setupSystemPll(const IRegisters<uint16_t>::BatchType regs[], uint16_t count) override;

    void startImager() override;
    void stopImager() override;


private:
};
