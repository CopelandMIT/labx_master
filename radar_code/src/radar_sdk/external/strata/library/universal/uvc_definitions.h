/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#ifndef UVC_DEFINITIONS_H
#define UVC_DEFINITIONS_H 1

/* UVC Extension Unit specification */

#define UVC_XU_GUID                     0x24, 0xA6, 0x00, 0xEF, 0x60, 0xA3, 0x16, 0x44, 0x7D, 0x94, 0x0A, 0x1C, 0xAE, 0x3B, 0xD0, 0xF8 // {ef00a624-a360-4416-947d-0a1cae3bd0f8}
#define UVC_XU_CONTROLS                 6
#define UVC_XU_CONTROL_BITMAP           0x3f,0x00,0x00


/* UVC Extension Unit property numbers used for vendor commands */

#define UVC_XU_PROPERTY_SETUP           1   /* 8 bytes
                                               Write to issue a command, if applicable followed by a read or write to one of the data properties.
                                               Read to retrieve the error code for the previous command.
                                               */
#define UVC_XU_PROPERTY_DATA_64         2   // always exactly 64 bytes of data (will be truncated according to wLength)
#define UVC_XU_PROPERTY_DATA_512        3   // always exactly 512 bytes of data (will be truncated according to wLength)
#define UVC_XU_PROPERTY_DATA_4096       4   // always exactly 4096 bytes of data (will be truncated according to wLength)
#define UVC_XU_PROPERTY_DATA_VARIABLE   5   // the amount of data indicated by wLength (up to DATA_BUFFER_LENGTH bytes, usually 4096. May not be supported by all host implementations!)
#define UVC_XU_PROPERTY_DATA_SETUP_56_8 6   // combined 8 byte setup packet at the end (with wLength <= 56) and data packet (this saves one additional separate transaction for lower payload write commands)


/* Status flags for UVC Extension Unit commands */

#define UVC_XU_REQ_COMPLETE             0x00
#define UVC_XU_REQ_ACTIVE               0x01
#define UVC_XU_SETUP_DONE               0x02
#define UVC_XU_SETUP_INCOMPLETE         0x03
#define UVC_XU_SETUP_BAD_LENGTH         0x04
#define UVC_XU_SETUP_INVALID            0x05
#define UVC_XU_DATA_DONE                0x06

#endif /* UVC_DEFINITIONS_H */
