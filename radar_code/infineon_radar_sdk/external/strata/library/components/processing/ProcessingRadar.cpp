/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "ProcessingRadar.hpp"

void ProcessingRadar::configure(uint8_t /*dataSource*/, const IDataProperties_t * /*dataProperties*/, const IProcessingRadarInput_t * /*radarInfo*/,
                                const IfxRsp_Stages * /*stages*/, const IfxRsp_AntennaCalibration * /*antennaConfig*/)
{
}

void ProcessingRadar::doFft(const IfxRsp_Signal * /*input*/, const IfxRsp_FftSetting * /*settings*/, IfxRsp_Signal * /*output*/, uint16_t /*samples*/, uint16_t /*offset*/, uint8_t /*dimension*/, uint8_t /*format*/)
{
}

void ProcessingRadar::doNci(const IfxRsp_Signal * /*input*/, uint8_t /*format*/, IfxRsp_Signal * /*output*/)
{
}

void ProcessingRadar::doThresholding(const IfxRsp_Signal * /*input*/, uint8_t /*dimension*/, const IfxRsp_ThresholdingSetting * /*settings*/, IfxRsp_Signal * /*output*/)
{
}

void ProcessingRadar::doPsd(const IfxRsp_Signal * /*input*/, uint16_t /*nFft*/, IfxRsp_Signal * /*output*/)
{
}

void ProcessingRadar::start()
{
}

bool ProcessingRadar::isBusy()
{
    return false;
}

void ProcessingRadar::writeConfigRam(uint16_t /*offset*/, uint16_t /*count*/, const uint32_t /*ramContent*/[])
{
}

void ProcessingRadar::writeCustomWindowCoefficients(uint8_t /*slotNr*/, uint16_t /*offset*/, uint16_t /*count*/, const uint32_t /*coefficients*/[])
{
}

void ProcessingRadar::reinitialize()
{
}
