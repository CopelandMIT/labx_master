/* ===========================================================================
** Copyright (C) 2023 Infineon Technologies AG
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** 1. Redistributions of source code must retain the above copyright notice,
**    this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. Neither the name of the copyright holder nor the names of its
**    contributors may be used to endorse or promote products derived from
**    this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
** ===========================================================================
*/


/**
 * @file DeviceLtr11Impl.hpp
 *
 * \brief \copybrief gr_device_ltr11
 *
 * For details refer to \ref gr_device_ltr11
 */

#pragma once

#include "DeviceLtr11Base.hpp"
#include "DeviceLtr11RegisterConfigurator.hpp"

// strata
#include <components/interfaces/IRegisters.hpp>
#include <platform/BoardInstance.hpp>
#include <universal/types/DataSettingsBgtRadar.h>

#include <atomic>
#include <memory>
#include <optional>

// forward declarations
class IRadarLtr11;
class IProtocolLtr11;


struct DeviceLtr11 : public DeviceLtr11Base
{
public:
    NONCOPYABLE(DeviceLtr11);

    DeviceLtr11(std::unique_ptr<BoardInstance>&& board);
    ~DeviceLtr11() override;

    void setConfig(const ifx_Ltr11_Config_t* config) override;

    void startAcquisition() override;
    void stopAcquisition() override;

    void dumpRegisters(const char* filename) override;

    ifx_Vector_C_t* getNextFrame(ifx_Vector_C_t* frame, ifx_Ltr11_Metadata_t* metadata, uint16_t timeoutMs) override;

private:
    void softReset();

    uint32_t determineBufferSize() const;

    void setupFrameData();
    void setupBridgeData();

    void setMisoArbitration(uint16_t prt = 0);

    void startDataStreaming();
    void stopDataStreaming();

    uint16_t getNumberOfSamples() const;

    void readNextFrame(ifx_Vector_C_t* frameData, ifx_Ltr11_Metadata_t* metadata, uint16_t timeoutMs);

    std::unique_ptr<BoardInstance> m_board;

    IRadarLtr11* m_radarLtr11;
    IBridgeData* m_bridgeData;
    IData* m_data;

    IProtocolLtr11* m_protocol;
    IRegisters<uint8_t, uint16_t>* m_registers;

    std::unique_ptr<DeviceLtr11RegisterConfigurator> m_registerConfigurator;

    uint8_t m_dataIndex;

    uint32_t m_frameSize;

    std::optional<uint64_t> m_timestampPrev;

    std::atomic<bool> m_acquisitionStarted;

    float m_frameCounter;
    float m_averagePower;
};
