/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#ifndef PROTOCOL_DEFINITIONS_H
#define PROTOCOL_DEFINITIONS_H 1

#define PROTOCOL_VERSION_MAJOR          4
#define PROTOCOL_VERSION_MINOR          0


/* Values for bmReqType */
#define VENDOR_REQ_WRITE                0x40    // write commands - Direction: Host to Device (OUT), Type: Vendor, Recipient: Device
#define VENDOR_REQ_READ                 0xC0    // read commands - Direction: Device to Host (IN), Type: Vendor, Recipient: Device
#define VENDOR_REQ_TRANSFER             0x43    // transfer commands - Direction: Host to Device (OUT), Type: Vendor, Recipient: Other
#define VENDOR_REQ_TRANSFER_2           0xC3    // transfer commands - Direction: Device to Host (IN), Type: Vendor, Recipient: Other

#define VENDOR_REQ_READ_LEGACY          0xC1    // read commands - Direction: Device to Host (IN), Type: Vendor, Recipient: Interface
#define VENDOR_REQ_WRITE_LEGACY         0x41    // write commands - Direction: Host to Device (OUT), Type: Vendor, Recipient: Interface
#define VENDOR_REQ_TRANSFER_LEGACY      0xE1    // transfer commands - Data is transferred from host to device and other data back, or data with unknown length is requested

#define DATA_FRAME_PACKET               0xD0    // data frame

#define DATA_FRAME_FLAG_FIRST           (1u << 0)  // first packet of a data frame
#define DATA_FRAME_FLAG_LAST            (1u << 1)  // last packet of a data frame
#define DATA_FRAME_FLAG_TIMESTAMP       (1u << 2)  // the last 8 bytes of the packet are a time stamp
#define DATA_FRAME_FLAG_ERROR           (1u << 3)  // the frame contains an error code instead of a payload

#define DATA_FRAME_SINGLE_PACKET        (DATA_FRAME_PACKET | DATA_FRAME_FLAG_FIRST | DATA_FRAME_FLAG_LAST)  // data frame - in one single packet
#define DATA_FRAME_FIRST_PACKET         (DATA_FRAME_PACKET | DATA_FRAME_FLAG_FIRST)                         // data frame - first packet of several
#define DATA_FRAME_MIDDLE_PACKET        (DATA_FRAME_PACKET)                                                 // data frame - middle packet of several
#define DATA_FRAME_LAST_PACKET          (DATA_FRAME_PACKET | DATA_FRAME_FLAG_LAST)                          // data frame - last packet of several


/* Values for bStatus */
#define STATUS_SUCCESS                  0x00    // == E_SUCCESS, can otherwise contain an E_* error code from error_definition.h

/* Errors occurring while parsing an incoming data packet */
#define STATUS_HEADER_INCOMPLETE        0x80
#define STATUS_PAYLOAD_INCOMPLETE       0x81
#define STATUS_PAYLOAD_TOO_LONG         0x82
#define STATUS_PAYLOAD_FRAGMENTED       0x83
#define STATUS_CRC_ERROR                0x84

/* Errors occurring while parsing a request out of a data packet*/
#define STATUS_REQUEST_TYPE_INVALID     0x85
#define STATUS_REQUEST_INVALID          0x86

/* Errors regarding the contents of a request and its execution */
#define STATUS_REQUEST_NOT_IMPLEMENTED  0x87
#define STATUS_REQUEST_WLENGTH_INVALID  0x88
#define STATUS_REQUEST_WVALUE_INVALID   0x89
#define STATUS_REQUEST_WINDEX_INVALID   0x8A
#define STATUS_REQUEST_PAYLOAD_INVALID  0x8B
#define STATUS_REQUEST_NOT_AVAILABLE    0x8C

/* Status values for Component / Module commands parsing */
#define STATUS_COMMAND_ID_INVALID       0x8D
#define STATUS_COMMAND_TYPE_INVALID     0x8E    // Module or component type invalid
#define STATUS_COMMAND_SUBIF_INVALID    0x8F    // Sub interface invalid for component or module
#define STATUS_COMMAND_FUNCTION_INVALID 0x90    // Function invalid for component, module or sub interface
#define STATUS_COMMAND_NOT_IMPLEMENTED  STATUS_REQUEST_NOT_IMPLEMENTED
#define STATUS_COMMAND_WLENGTH_INVALID  STATUS_REQUEST_WLENGTH_INVALID
#define STATUS_COMMAND_PAYLOAD_INVALID  STATUS_REQUEST_PAYLOAD_INVALID


/* Values for bRequest */

/*********************************************************************************************************************/
/************************************************* Platform requests *************************************************/
/*********************************************************************************************************************/

#define REQ_BOARD_INFO                  0x80
// write - Manage bootloader
#define REQ_BOARD_INFO_BOOTLOADER_WVALUE        0x04  /* enter bootloader mode
                                                         - if not supported, an error will be returned
                                                         - if supported, usually a protocol timeout will occur,
                                                         after which the transport protocol can be tried (e.g. USB-CDC)
                                                       */
// read  - Read out board info
#define REQ_BOARD_INFO_VERSION_INFO_WVALUE      0x01  /* version information about firmware/software and protocol
                                                          wLength: 16
                                                          payload: eight 16bit values {major, minor, patch, build, protocolMajor, protocolMinor, reserved1, reserved2}
                                                       */
#define REQ_BOARD_INFO_UUID_WVALUE              0x02  /* UUID of the board (unique identification of the board to destinguish boards of the same type)
                                                          wLength: 16
                                                          payload: UUID, right aligned, 0-padded if shorter
                                                       */
// transfer - Read out board info
#define REQ_BOARD_INFO_BOARD_INFO_WVALUE        0x00  /* board information (does not have to be implemented on USB devices)
                                                          wLength: variable
                                                          payload: 16bit VID, 16bit PID, char name[]
                                                       */
#define REQ_BOARD_INFO_EXTENDED_VERSION_WVALUE  0x03  /* extended version information. If provided by firmware this string can contain additional version information
                                                         such as commit hash, tag information, build date and further meta information. The format of the string
                                                         is unspecified and is up to the firmware developer.
                                                         wLength: variable
                                                         payload: char extendedVersion[]
                                                       */
// read  - Read out error info
#define REQ_BOARD_INFO_ERROR_INFO_WVALUE        0xFF  /* error information
                                                         (does not have to be implemented by protocols that have a native error signalling) */
#define REQ_BOARD_INFO_ERROR_INFO_LAST_ERROR_WINDEX     0x01  /* last error: status packet for protocols that have no explicit error checking
                                                                 (then this is mandatory to check if write commands succeeded, has to be implemented on USB devices)
                                                                 otherwise not implemented
                                                                 wLength: 4
                                                                 payload: bmReqType, bStatus, wLength
                                                               */
#define REQ_BOARD_INFO_ERROR_INFO_DETAILED_ERROR_WINDEX 0x02  /* detailed error: read out platform specific detailed error info
                                                                 wLength: usually (count * 4)
                                                                 payload: usually 32bit values
                                                               */


#define REQ_MEMORY                      0x00    /* write/read - access device memory (does not have to be implemented)
                                                        wValue: 32bit address LOW word
                                                        wIndex: 32bit address HIGH word
                                                        wLength: byte count
                                                        payload: byte data
                                                   */

#define REQ_GPIO                        0x01    /* write - set GPIO to a certain logical output level or configure it
                                                        wValue: 0 for low, 1 for high, ((flags << 8) | 2) for configure
                                                        wIndex: GPIO number or name
                                                        wLength: 0
                                                   read  - get GPIO logical input level
                                                        wIndex: GPIO number or name
                                                        wLength: 1
                                                        payload: 0 for low, 1 for high
                                                   */

#define REQ_PORT                        0x02    /* write - set multiple port pins to certain logical output levels or configure them
                                                        wValue: 1 for setting output levels, ((flags << 8) | 2) for configure
                                                        wIndex: Port number
                                                        wLength: sizeof(Port) for configure, 2 * sizeof(Port) for set
                                                        payload: mask for configure, {mask, states} for set
                                                   read  - get Port input level
                                                        wIndex: Port number
                                                        wLength: sizeof(Port)
                                                        payload: pin states of the whole port
                                                   */

#define REQ_I2C                         0x03    /*
                                                    - write: with transaction bit set to zero configures the bus speed, or clears the bus if speed == 0
                                                        wValue: (busId << 12) | (12 MSB of clock speed)
                                                        wIndex: (0 << 15) | (15 LSB of clock speed)
                                                        wLength: 0
                                                    - write/read: to the device address given in I2C address
                                                        wValue: I2C address (incl. busId)
                                                        wIndex: REQ_I2C_FLAG_TRANSACTION | (0 / REQ_I2C_FLAG_WITH_REG_ADDR) | (prefix)
                                                        wLength: byte count
                                                        payload: byte data
                                                    - write: poll ack from the device address given in I2C address
                                                        wValue: I2C address (incl. busId)
                                                        wIndex: REQ_I2C_FLAG_TRANSACTION | REQ_I2C_FLAG_ACK_POLLING
                                                        wLength: 0
                                                    - write: with config default set, sets the given I2C address as default device address
                                                        wValue: I2C address (incl. busId)
                                                        wIndex: REQ_I2C_FLAG_TRANSACTION | REQ_I2C_FLAG_CONFIGURE_DEFAULT
                                                        wLength: 0
                                                    - read: with config default set, gets the I2C address used as default device address
                                                        wValue: 0
                                                        wIndex: REQ_I2C_FLAG_TRANSACTION | REQ_I2C_FLAG_CONFIGURE_DEFAULT
                                                        wLength: 2
                                                        payload: I2C address (incl. busId)
                                                   */
    /* REQ_I2C wValue flags */
    #define REQ_I2C_FLAG_TRANSACTION        (1u << 15)  // execute transaction or configure default device address
    #define REQ_I2C_FLAG_CONFIGURE_DEFAULT  (1u << 14)  // configure default device address
    #define REQ_I2C_FLAG_ACK_POLLING        (1u << 9)   // poll for ACK from device address
    #define REQ_I2C_FLAG_WITH_REG_ADDR      (1u << 8)   // 8bit transaction with register address

#define REQ_I2C_TRANSACTION_16          0x04    /* write/read - I2C transfer with a 16bit prefix
                                                        wValue: I2C address
                                                        wIndex: prefix
                                                        wLength: byte count
                                                        payload: byte data
                                                   */

#define REQ_SPI                         0x05    /*
                                                    - write - configure an SPI device
                                                        wValue:  (wordSize << 8) | (device ID)
                                                        The width of buffer elements could be 1, 2 or 4 bytes.
                                                        wIndex:  (0 << 15) | flags
                                                        wLength: 4
                                                        payload: 32bit clock speed
                                                    - write/read/transfer - exchange raw byte stream with an SPI device with a buffer width of 1, 2 or 4 bytes
                                                        wValue:  (buffer width << 8) | (device ID)
                                                        wIndex:  REQ_SPI_FLAG_TRANSACTION | (0 / REQ_SPI_FLAG_KEEP_SELECTED)
                                                        wLength: byte count
                                                        payload: byte data
                                                   */
    /* REQ_SPI wValue flags */
    #define REQ_SPI_FLAG_TRANSACTION        (1u << 15)  // execute transaction
    #define REQ_SPI_FLAG_KEEP_SELECTED      (1u << 14)  // keep device selected after the transaction

#define REQ_FLASH_TRANSACTION           0x06    /* write/read - Access (SPI) flash with (24bit) address
                                                                (must be page-wise for write and segment-wise for read (read restriction only if applicable))
                                                        wValue: (24bit address LOW byte << 8) | (device ID)
                                                        wIndex: 24bit address HIGH bytes
                                                        wLength: byte count
                                                        payload: byte data
                                                   */
#define REQ_FLASH_ERASE_WIP             0x07    /* write - Erase sector at given (24 bit) address
                                                        wValue: (24bit address LOW byte << 8) | (device ID)
                                                        wIndex: 24bit address HIGH bytes
                                                        wLength: 0
                                                   read  - WIP (write-in-progress) bit
                                                        wLength: 1
                                                        payload: 1bit WIP
                                                   */
#define REQ_FLASH_COMMAND_SELECT        0x08    /* write - set 8bit SPI slave device ID used for custom command
                                                        wValue: 8bit device ID
                                                        wLength: 0
                                                   read  - get 8bit SPI slave device ID used for custom command
                                                        wLength: 1
                                                        payload: 8bit device ID
                                                    */
#define REQ_FLASH_COMMAND               0x09    /* write/read - send custom command to (SPI) Flash
                                                        wValue: (24bit address LOW byte << 8) | (command)
                                                        wIndex: 24bit address HIGH bytes
                                                        wLength: byte count
                                                        payload: byte data
                                                    */

#define REQ_DATA                        0x0D    /* read/write - wIndex: data interface id
                                                                wValue, wLength: see defines below
                                                   */
    /* REQ_DATA wValue for write */
    #define REQ_DATA_CONFIGURE          0x01    // wLength: serialized buffer lengths
    #define REQ_DATA_START              0x03    // wLength: 0
    #define REQ_DATA_STOP               0x04    // wLength: 0

    /* REQ_DATA wValue for read */
    #define REQ_DATA_STATUS_FLAGS       0x00    // wLength: 4, payload: 32bit status flags


#define REQ_MEMORY_STREAM               0x0E    /* write - stream memory content (does not have to be implemented)
                                                        wValue: 32bit destination address LOW word
                                                        wIndex: 32bit destination HIGH word
                                                        wLength: 8
                                                        payload: 32bit source address
                                                                 32bit length
                                                   */

#define REQ_FPGA_PROGRAM                0x0F    /* write - send FPGA configuration data (does not have to be implemented)
                                                        wValue: 32bit total count LOW word for first chunk, zero for following
                                                        wIndex: 32bit total count HIGH word for first chunk, zero for following
                                                        wLength: chunk length
                                                        payload: binary data
                                                   read  - get done pin
                                                        wLength: 1
                                                        payload: pin state
                                                   */


#define REQ_MACRO                       0x1F
#define REQ_MACRO_REQUEST_BUFFER_WVALUE         0x01    /* write - adds bytes to request buffer
                                                                 with wLength = 0: execute request buffer
                                                           read  - get total buffer size
                                                                 wLength: 4
                                                                 payload: 32bit buffer size */
#define REQ_MACRO_RESPONSE_BUFFER_WVALUE        0x02    /* transfer - reads bytes from the response buffer
                                                                 when wResponseLength = 0: all bytes received */
#define REQ_MACRO_EXTENDED_FUNC_WVALUE          0xFF    /* write - execute extended functionality according to wIndex */
#define REQ_MACRO_EXTENDED_FUNC_DELAY_WINDEX            0x01  /* wait a specific delay in microseconds
                                                                 wLength: 4
                                                                 payload: 32bit value
                                                               */

/*********************************************************************************************************************/
/******************************************** Module / component requests ********************************************/
/*********************************************************************************************************************/

/* For module and component requests the value and index fields contain exact information about which part of the board
   is addressed and which command shall be exectuted. The follwowing information is included:
   - type: Type of the module or component (depending on the command type). See modules/types.h and components/types.h for possible types
   - implementation: If there is a special implementation of the module or component type, this field specifies it. Specified in the same headers as type.
   - id: As there can be multiple modules or components of the same type, this is the id / index of the part to address.
   - subinterface: Which special interface of the module or component shall be addressed? See components/subinterfaces.h
   - function: Specifies the function / command to execute. The functions are defined separately for each (sub)module and (sub)component in separate header files.

   The following macros show the assembly of the fields.
   */
#define CMD_W_VALUE(type)                         (type)                                            /* Assemble value parameter from type and implementation */
#define CMD_W_INDEX(id, subInterface, function)   ((function) << 8 | ((subInterface) << 4) | (id))  /* Assemble index parameter from id, subinterface and function */
#define CMD_GET_TYPE(wValue)                      (wValue)                                          /* Extract module or component type from the value parameter */
#define CMD_GET_ID(wIndex)                        ((wIndex) & 0x0F)                                 /* Extract ID of the module / component from the index parameter */
#define CMD_GET_SUBIF(wIndex)                     (((wIndex) >> 4) & 0x0F)                          /* Extract sub interface type from the index parameter */
#define CMD_GET_FUNCTION(wIndex)                  ((wIndex) >> 8)                                   /* Extract function from the index parameter */

#define CMD_COMPONENT                   0x20    /* read/write - execute a component command */
#define CMD_MODULE                      0x21    /* read/write - execute a module command */

/*********************************************************************************************************************/
/******************************************** Custom / specific requests *********************************************/
/*********************************************************************************************************************/

#define REQ_CUSTOM                      0x30    /* read/write - execute a custom command
                                                        (see definition in protocol_definitions_custom.h)
                                                   */

#define CMD_MIRA                        0x40    /* read/write - execute a mira command (does not have to be implemented)
                                                        (see definition in protocol_definitions_mira.h)
                                                   */

#define CMD_CTRX                        0x50    /* read/write - execute a ctrx command (does not have to be implemented)
                                                        (see definition in protocol_definitions_ctrx.h)
                                                    */

#endif /* PROTOCOL_DEFINITIONS_H */
