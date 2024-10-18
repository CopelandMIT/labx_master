/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "protocol_definitions.h"

enum PsHeaderMode
{
  PsHeaderMode_HeaderWithPayload = 0x00,
  PsHeaderMode_HeaderOnly = 0x01,
  PsHeaderMode_Reserved = 0x02 /* deprecated and only here for backwards compatiblity */
};

enum CtrxDmuxPins
{
  CtrxDmuxPins_DMUX1 = 1,
  CtrxDmuxPins_DMUX2,
  CtrxDmuxPins_DMUX3
};

enum
{
  CtrxSystemInfo_Length = 0U,
  CtrxSystemInfo_String = 1U
};

enum CtrxBoardResetDevice
{
  CtrxBoardResetDevice_DphyController = 1U
};

/* write - configureReceiver
        wLength: 7 byte
        payload:
            - 8bit use LVDS receiver (true=1, false=0)
            - 8bit header options (0x0...send data with header, 0x1...send only header no data, 0x2...send only data no header)
            - 16bit sequence count
            - 16bit number of ramps
            - 8bit bit pattern enabling/disabling crc on specific LVDS lanes
    */
#define CMD_CTRX_CONFIGURE_RECEIVER    0x01

/* write - Activate JTAG
        wIndex: Mode
            - 0: Normal (no JTAG)
            - 1: Full JTAG via SPI (async)
            - 2: Full JTAG via SPI (sync)
            - 3: cJTAG and SPI
	        - 4: cJTAG via SPI
	        - 5: cJTAG via DMUX1&2
	        - 6: cJTAG via DMUX2&3
	        - 7: cJTAG via DMUX1&3
        wLength: 16
        payload: JTAG activation sequence
    */
#define CMD_CTRX_ACTIVATE_JTAG         0x02

/* write - Enable JTAG test mode to be able to access the Test Register Chain (TRX) via JTAG
        wIndex: 0
        wLength: 16
        payload: The password needed to activate the test mode
*/
#define CMD_CTRX_ENABLE_JTAG_TESTMODE  0x03

/* write - Write memory using the JTAG interface
        wIndex: 0
        wLength: 4 + (4 * number of values to write)
        Payload: Address (32bit) and values (each 32bit)
*/
#define CMD_CTRX_WRITE_MEMORY_VIA_JTAG 0x04

/* transfer - Read memory using the JTAG interface
        Must be a different ID than write since the firmware cannot distinguish between
        write and transfer calls
        wIndex: 0
        wLength In: 4 + 4
        payload In: Address (32bit) and number of values to read (32bit)
        wLength Out: 4 * number of values read
        payload Out: read values (each 32bit)
*/
#define CMD_CTRX_READ_MEMORY_VIA_JTAG  0x05

/* read - System Info
           Retrieves information on the setup of the board, including versions of the different components.
           The information is to be provided as JSON string.
        wIndex:
            0 (CtrxSystemInfo_Length) = retrieve the length of the string, wLength = 2 then
            1 (CtrxSystemInfo_String) = retrieve the string, without \0, wLength is what was retrieved before via wIndex=0
   */
#define CMD_CTRX_SYSTEM_INFO 0x06

/* write - Set LVDS Mode
           Prepare the LVDS receiver for data other than samples of ramp sequences.

           There is no distinct command to select the receiver and to configure the header options.
           These settings are embeededd in CMD_CTRX_CONFIGURE_RECEIVER and now also in this command.
           That way useres do not need to call CMD_CTRX_CONFIGURE_RECEIVER when receiving LVDS debug data as that would be confusing.
        wIndex: Mode
            - 0: Ramp Sequence Mode (default)
            - 1: Debug Data Mode
        wLength: 2 byte
        payload:
            - 8bit use LVDS receiver (true=1, false=0)
            - 8bit header options (0x0...send data with header, 0x1...send only header no data, 0x2...send only data no header)
    */
#define CMD_CTRX_LVDS_MODE 0x07

/* write - Record debug data via LVDS.
           You need to set the LVDS Mode accordingly first.
        wIndex: triggerMode
            - 0: Immediate software trigger (default), 1: HW trigger rising edge 2: HW trigger falling edge
        wLength: 5 byte
        payload:
            - 32bit number of samples per lane
            - 8bit operation mode
    */
#define CMD_CTRX_LVDS_RECORD_DEBUG_DATA 0x08

/* transfer - Read a complete data register identified by an opcode using the JTAG interface
        wIndex: 0
        wLength In: 1 + 2
        payload In: opCode (8Bit) and  number of bits to read (16Bit)
        wLength Out: number of values read based on the bits to read
        payload Out: read values (each 8bit)
*/
#define CMD_CTRX_READ_DATA_REGISTER_VIA_JTAG 0x0a

/* write - Write a complete data register identified by an opcode using the JTAG interface
        wIndex: 0
        wLength: 1 + 2 + (number of values to write)
        Payload: opCode (8Bit), number of bits to write(16Bit) and values (each 8bit)
*/
#define CMD_CTRX_WRITE_DATA_REGISTER_VIA_JTAG 0x0b

/* write - Calibrate the LVDS receiver.
           This starts a deskewing routine that determines the optimal clock delays.
        wLength: 0
    */
#define CMD_CTRX_CALIBRATE_LVDS_RECEIVER 0x0C

/* read - Last firmware command execution time
          Retrieves the time the last SPI firmware command execution took
        Payload: Execution time in us (32bit)
   */
#define CMD_CTRX_LAST_FWCMD_EXECUTION_TIME 0x0D

/* write - configureReceiverParam
        wLength: 21 byte
        payload:
            - 8bit use LVDS receiver (true=1, false=0)
            - 8bit header options (0x0...send data with header, 0x1...send only header no data, 0x2...send only data no header)
            - 4 * 16bit sequence count
            - 4 * 16bit number of ramps
            - 8bit bit pattern enabling/disabling crc on specific LVDS lanes
            - 8bit bit pattern enabling/disabling csi2 active lanes
            - 8bit triggerMode, 0: Immediate software trigger (default), 1: HW trigger rising edge 2: HW trigger falling edge
    */
#define CMD_CTRX_CONFIGURE_RECEIVER_PARAM    0x0E

/* write - reset
           Performs reset of the device specified with wIndex.

        wLength: 0
        wIndex: Device to reset. See the CtrxBoardResetDevices enum above for possible values.
   */
#define CMD_CTRX_RESET 0x0F

/* write - Switch between LVDS and CSI2 mode.
        wLength: 1
        wIndex: 8bit use LVDS receiver (true=1, false=0)
    */
#define CMD_CTRX_USE_LVDS_RECEIVER 0x10

/* write - Benchmark
           Starts benchmark mode, i.e. sends sequence count sequences of rampCount x sampleCount of random data.

        wIndex: 0
        wLength: 12
        payload:  - 32bit sequence count
                  - 32bit ramp count
                  - 32bit sample count
   */
#define CMD_CTRX_BENCHMARK 0x11

/* transfer -  Fetches one warning or error log from a FIFO queue on the firmware
               Logs can be lost depending on the FIFO size, the frequency of logging and the frequency
               the logs are fetched.

            wLength: the maximum number of bytes (including '\0') to receive
   */
#define CMD_CTRX_FIRMWARE_FETCH_LOG 0x12

/* write - Set CTRX Chip Type.
        wLength: 1
        wIndex: 0
        payload: Chiptype (A, B, C)
    */
#define CMD_CTRX_SET_CHIPTYPE 0x13

/* write - Enable Jtag Gpio Pulse.
        wLength: 3
        wIndex: 0
        payload: - 8bit enable
                 - 8bit opCode
                 - 8bit idxDmux
    */
#define CMD_CTRX_ENABLE_JTAG_GPIO_PULSE 0x14
