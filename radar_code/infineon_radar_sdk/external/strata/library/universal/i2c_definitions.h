/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#ifndef I2C_DEFINITIONS_H
#define I2C_DEFINITIONS_H 1


#define I2C_BUS_ID(busId)               ((busId) << 12)
#define I2C_ADDR(busId, devAddr)        (I2C_BUS_ID(busId) | (devAddr))
#define I2C_10_BIT_ADDR(busId, devAddr) (I2C_BUS_ID(busId) | I2C_10_BIT_DEV_ADDR | (devAddr))

#define I2C_DEFAULT_DEVICE              (1u << 11)   ///< Use the defined default device address
#define I2C_10_BIT_DEV_ADDR             (1u << 10)   ///< Use an extended 10bit device address

#ifdef __cplusplus
#define GET_I2C_BUS_ID_VALUE(devAddr)   static_cast<uint8_t>((devAddr) >> 12)
#else
#define GET_I2C_BUS_ID_VALUE(devAddr)   (uint8_t)(devAddr >> 12)
#endif
#define GET_I2C_DEV_ADDR_10BIT(devAddr) ((devAddr) & 0x03FFu)
#define GET_I2C_DEV_ADDR_7BIT(devAddr)  ((devAddr) & 0x007Fu)

#endif /* I2C_DEFINITIONS_H */
