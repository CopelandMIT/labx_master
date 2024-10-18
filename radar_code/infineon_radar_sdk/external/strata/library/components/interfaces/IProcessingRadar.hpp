/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "IComponent.hpp"
#include <universal/components/processing.h>
#include <universal/data_definitions.h>
#include <universal/types/IfxRsp_Types.h>


typedef struct _IProcessingRadarInput_t
{
    uint8_t txChannels;   ///< no. of transmit channels
    uint8_t virtualAnt;   ///< no. of virtual antennae
    uint16_t rampsPerTx;  ///< no. of ramps per transmit channel

    float maxRange;
    float maxVelocity;

} IProcessingRadarInput_t;


class IProcessingRadar : public IComponent
{
public:
    static constexpr uint16_t getType()
    {
        return COMPONENT_TYPE_PROCESSING_RADAR;
    }

    virtual ~IProcessingRadar() = default;

    virtual void configure(uint8_t dataSource, const IDataProperties_t *dataProperties, const IProcessingRadarInput_t *radarInfo, const IfxRsp_Stages *stages,
                           const IfxRsp_AntennaCalibration *antennaConfig = nullptr) = 0;

    virtual void doFft(const IfxRsp_Signal *input, const IfxRsp_FftSetting *settings, IfxRsp_Signal *output, uint16_t samples, uint16_t offset, uint8_t dimension, uint8_t format) = 0;

    virtual void doNci(const IfxRsp_Signal *input, uint8_t format, IfxRsp_Signal *output) = 0;

    virtual void doThresholding(const IfxRsp_Signal *input, uint8_t dimension, const IfxRsp_ThresholdingSetting *settings, IfxRsp_Signal *output) = 0;

    virtual void doPsd(const IfxRsp_Signal *input, uint16_t nFft, IfxRsp_Signal *output) = 0;

    virtual void writeConfigRam(uint16_t offset, uint16_t count, const uint32_t ramContent[]) = 0;

    virtual void writeCustomWindowCoefficients(uint8_t slotNr, uint16_t offset, uint16_t count, const uint32_t coefficients[]) = 0;

    virtual void reinitialize(void) = 0;

    virtual void start() = 0;

    virtual bool isBusy() = 0;
};
