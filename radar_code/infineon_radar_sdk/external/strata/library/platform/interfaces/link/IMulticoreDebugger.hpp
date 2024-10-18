/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/access/IMemory.hpp>


class IMulticoreDebugger
{
public:
    virtual ~IMulticoreDebugger() = default;

    virtual IMemory<uint32_t> *getIMemory() = 0;

    virtual bool isOpened()             = 0;
    virtual void open(uint8_t i_system) = 0;
    virtual void close()                = 0;

    virtual uint8_t getCore()                     = 0;
    virtual bool isRunning()                      = 0;
    virtual void reset(bool halt)                 = 0;
    virtual void run()                            = 0;
    virtual void stop()                           = 0;
    virtual void setFrequency(uint32_t frequency) = 0;
};
