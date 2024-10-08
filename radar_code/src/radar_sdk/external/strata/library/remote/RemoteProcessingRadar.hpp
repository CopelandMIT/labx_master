/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "RemoteVendorCommands.hpp"
#include <components/interfaces/IProcessingRadar.hpp>
#include <platform/interfaces/IVendorCommands.hpp>


class RemoteProcessingRadar :
    public IProcessingRadar
{
public:
    RemoteProcessingRadar(IVendorCommands *commands, uint8_t id);

    void configure(uint8_t dataSource, const IDataProperties_t *dataProperties, const IProcessingRadarInput_t *radarInfo, const IfxRsp_Stages *stages,
                   const IfxRsp_AntennaCalibration *antennaConfig) override;
    void doFft(const IfxRsp_Signal *input, const IfxRsp_FftSetting *settings, IfxRsp_Signal *output, uint16_t samples, uint16_t offset, uint8_t dimension, uint8_t format) override;
    void doNci(const IfxRsp_Signal *input, uint8_t format, IfxRsp_Signal *output) override;
    void doThresholding(const IfxRsp_Signal *input, uint8_t dimension, const IfxRsp_ThresholdingSetting *settings, IfxRsp_Signal *output) override;
    void doPsd(const IfxRsp_Signal *input, uint16_t nFft, IfxRsp_Signal *output) override;
    void writeConfigRam(uint16_t offset, uint16_t count, const uint32_t values[]) override;
    void writeCustomWindowCoefficients(uint8_t slotNr, uint16_t offset, uint16_t count, const uint32_t coefficients[]) override;
    void reinitialize() override;
    void start() override;
    bool isBusy() override;

private:
    RemoteVendorCommands m_commands;
};
