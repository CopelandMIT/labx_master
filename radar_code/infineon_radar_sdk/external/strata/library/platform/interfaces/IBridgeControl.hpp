/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/IVendorCommands.hpp>
#include <platform/interfaces/access/IData.hpp>
#include <platform/interfaces/access/IFlash.hpp>
#include <platform/interfaces/access/IGpio.hpp>
#include <platform/interfaces/access/II2c.hpp>
#include <platform/interfaces/access/IMemory.hpp>
#include <platform/interfaces/access/ISpi.hpp>

#include <array>
#include <string>


class IBridgeControl
{
public:
    virtual ~IBridgeControl() = default;

    using BoardInfo_t   = std::array<uint8_t, 256>;
    using VersionInfo_t = std::array<uint16_t, 8>;
    using Uuid_t        = std::array<uint8_t, 16>;

    /**
     * If this Bridge contains an IVendorCommands interface, it returns a valid pointer
     * This function is introduced to avoid dynamic_cast in some places
     * Should be removed as soon as no longer used since it's bad object-oriented design.
     */
    virtual IVendorCommands *getIVendorCommands() = 0;
    virtual void checkVersion()                   = 0;

    /**
     * Read out the board info during enumeration.
     * (This function is called during enumeration
     *  and is not intended to be called by the user)
     *
     * @param buffer a caller-provided buffer to directly read into
     */
    virtual void getBoardInfo(BoardInfo_t &buffer) = 0;

    /**
     * Returns a reference to the version info of the connected board
     */
    virtual const VersionInfo_t &getVersionInfo() = 0;

    /**
     * Returns a reference to a string built from the version info
     */
    virtual const std::string &getVersionString() = 0;

    /**
     * Returns a reference to a string containing more detailed version
     * information as given by the connected board
     */
    virtual const std::string &getExtendedVersionString() = 0;

    /**
     * Returns a reference to an array containing the UUID of the connected board
     */
    virtual const Uuid_t &getUuid() = 0;

    /**
     * Returns a reference to an array containing the UUID of the connected board
     */
    virtual const std::string &getUuidString() = 0;

    /**
     * Activates the bootloader of the board to allow updating the firmware.
     * (From then on, usually a proprietary protoocl has to be used)
     */
    virtual void activateBootloader() = 0;

    virtual void setDefaultTimeout()        = 0;
    virtual uint16_t getMaxTransfer() const = 0;

    virtual IData *getIData()               = 0;
    virtual IGpio *getIGpio()               = 0;
    virtual II2c *getII2c()                 = 0;
    virtual ISpi *getISpi()                 = 0;
    virtual IFlash *getIFlash()             = 0;
    virtual IMemory<uint32_t> *getIMemory() = 0;
};
