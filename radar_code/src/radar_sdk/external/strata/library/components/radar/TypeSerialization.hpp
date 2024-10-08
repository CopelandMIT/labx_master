/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "TypeSerializationSize.hpp"


uint8_t *hostToSerial(uint8_t buf[], const IfxRfe_MmicConfig *c);
const uint8_t *serialToHost(const uint8_t buf[], IfxRfe_MmicConfig *c);

uint8_t *hostToSerial(uint8_t buf[], const IfxRfe_Ramp r[], uint8_t rampCount);
uint8_t *hostToSerial(uint8_t buf[], const IfxRfe_Sequence *s);
const uint8_t *serialToHost(const uint8_t buf[], IfxRfe_Ramp r[], uint8_t rampCount);
const uint8_t *serialToHost(const uint8_t buf[], IfxRfe_Sequence *s);

uint8_t *hostToSerial(uint8_t buf[], const IDataProperties_t *c);
const uint8_t *serialToHost(const uint8_t buf[], IDataProperties_t *c);

uint8_t *hostToSerial(uint8_t buf[], const IProcessingRadarInput_t *c);
const uint8_t *serialToHost(const uint8_t buf[], IProcessingRadarInput_t *c);

uint8_t *hostToSerial(uint8_t buf[], const IfxRsp_AntennaCalibration *c);
const uint8_t *serialToHost(const uint8_t buf[], IfxRsp_AntennaCalibration *c);

uint8_t *hostToSerial(uint8_t buf[], const IfxRsp_Stages *c);
const uint8_t *serialToHost(const uint8_t buf[], IfxRsp_Stages *c);

uint8_t *hostToSerial(uint8_t buf[], const IfxRsp_Signal *s);
const uint8_t *serialToHost(const uint8_t buf[], IfxRsp_Signal *s);

uint8_t *hostToSerial(uint8_t buf[], const IfxRsp_FftSetting *c);
const uint8_t *serialToHost(const uint8_t buf[], IfxRsp_FftSetting *c);

uint8_t *hostToSerial(uint8_t buf[], const IfxRsp_ThresholdingSetting *c);
const uint8_t *serialToHost(const uint8_t buf[], IfxRsp_ThresholdingSetting *c);

uint8_t *hostToSerial(uint8_t buf[], const IfxRsp_DetectionSettings *c);
const uint8_t *serialToHost(const uint8_t buf[], IfxRsp_DetectionSettings *c);
