/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/Memory.hpp>
#include <platform/interfaces/link/IMulticoreDebugger.hpp>

#include <mcd_loader_class/mcd_loader_class.h>


class MulticoreDebugger :
    public IMulticoreDebugger,
    public Memory<uint32_t>
{
public:
    MulticoreDebugger();
    ~MulticoreDebugger() override;

    bool isOpened() override;
    void open(uint8_t i_system) override;
    void close() override;

    //IMulticoreDebugger
    IMemory<uint32_t> *getIMemory() override;

    uint8_t getCore() override;
    bool isRunning() override;
    void reset(bool halt) override;
    void run() override;
    void stop() override;
    void setFrequency(uint32_t frequency) override;

    uint32_t getMaxPayload() const;

private:
    //IMemory
    uint32_t read(uint32_t address) override;
    void read(uint32_t address, uint32_t &value) override;
    void write(uint32_t address, uint32_t value) override;
    void read(uint32_t address, uint32_t count, uint32_t data[]) override;
    void write(uint32_t address, uint32_t count, const uint32_t data[]) override;
    void writeBatch(const BatchType vals[], uint32_t count, bool optimize = false) override;

    void read16(uint32_t address, uint16_t &value);
    void write16(uint32_t address, uint16_t value);
    void setMem(uint32_t address, uint32_t value, uint32_t count);

private:
    McdLoaderClass m_mcdInstance;
    mcd_server_st *m_server;
    mcd_core_st *m_core;
    uint32_t m_maxPayload;

    mcd_tx_st m_tx;
    mcd_txlist_st m_txlist;
};
