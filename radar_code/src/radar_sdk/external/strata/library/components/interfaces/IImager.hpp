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
#include <components/interfaces/IRegisters.hpp>


class IImager
{
public:
    using BatchType = IRegisters<uint16_t>::BatchType;

    virtual ~IImager() = default;

    virtual IRegisters<uint16_t> *getIRegisters() = 0;
    virtual IPinsIrs *getIPinsIrs()               = 0;

    virtual void startImager() = 0;
    virtual void stopImager()  = 0;

    virtual void reset()                                                = 0;
    virtual void setupSystemPll(const BatchType regs[], uint16_t count) = 0;
    virtual void configure(const BatchType regs[], uint16_t count)      = 0;

    template <std::size_t N>
    inline void setupSystemPll(const BatchType (&regs)[N])
    {
        setupSystemPll(regs, N);
    }

    template <std::size_t N>
    inline void configure(const BatchType (&regs)[N])
    {
        configure(regs, N);
    }
};
