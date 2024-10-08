/* ===========================================================================
** Copyright (C) 2022 Infineon Technologies AG
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

#include <cstring>
#include <functional>
#include <mutex>
#include <vector>

#include "ifxBase/Uuid.h"
#include "internal/RadarDeviceCommon.hpp"
#include "platform/exception/EProtocolFunction.hpp"
#include "RadarDeviceCommon.h"

#include <platform/BoardManager.hpp>
#include <platform/exception/EConnection.hpp>

#include <components/interfaces/IRadarAtr22.hpp>
#include <components/interfaces/IRadarAvian.hpp>
#include <components/interfaces/IRadarLtr11.hpp>
#include <components/nonvolatileMemory/NonvolatileMemoryEepromI2c.hpp>

/* We use the function Infineon::Avian::detect_device_type to determine
 * what kind of radar sensor of the Avian family is used. This, unfortunately,
 * adds a dependency to Avian. But copying the logic from Avian would be even
 * worse.
 * If in case we need to break this dependency in the future, we can add #ifdefs.
 */
#include <ifxAvian_Utilities.hpp>

using namespace rdk::RadarDeviceCommon;


namespace {

constexpr bool use_serial = true;  // including USB CDC
constexpr bool use_ethernet = true;
constexpr bool use_uvc = false;
constexpr bool use_wiggler = false;
constexpr bool use_libusb = false;

/* board_manager may be used by multiple threads concurrently. Each access to
 * board_manager must be protected using the mutex mutex_board_manager. This
 * presents "weird" behavior like that some boards are not found if you
 * enumerate the boards from two different threads at the same time.
 */
std::mutex mutex_board_manager;


ifx_Radar_Sensor_t get_avian_type(std::unique_ptr<BoardInstance>& board, uint8_t id = 0)
{
    auto* avian = board->getComponent<IRadarAvian>(id);

    // set the correct SPI high-speed compensation,
    // so that we can read the chip id register
    try
    {
        avian->getIRegisters()->write(0x06, 0x100000);
    }
    catch (const EProtocolFunction& e)
    {
        // the function will fail if a FIFO overflow occured previously
        // the returned error flag is auto-cleared and the next call should succeed
        if (e.code() != E_UNEXPECTED_VALUE)
        {
            throw;
        }
    }

    const uint32_t chip_id_register = avian->getIRegisters()->read(0x02);
    return ifx_Radar_Sensor_t(Infineon::Avian::detect_device_type(chip_id_register));
}

bool get_sensor_type_legacy(std::unique_ptr<BoardInstance>& board, ifx_Radar_Sensor_t& sensor_type)  // LEGACY PROTOCOL
{
    uint8_t impl;
    board->getIVendorCommands()->vendorReadParameters(0x20, 0x0001, 0x0000, impl);
    const uint16_t type = 0x0100 | impl;

    if (type == IRadarAvian::getType())
    {
        sensor_type = get_avian_type(board);
        return true;
    }
    else if (type == IRadarAtr22::getType())
    {
        sensor_type = IFX_MIMOSE_BGT24ATR22;
        return true;
    }
    else if (type == IRadarLtr11::getType())
    {
        sensor_type = IFX_BGT60LTR11AIP;
        return true;
    }

    return false;
}

std::vector<ifx_Radar_Sensor_List_Entry_t> get_list(BoardManager& board_manager, rdk::RadarDeviceCommon::SelectorFunction&& selector)
{
    std::vector<ifx_Radar_Sensor_List_Entry_t> list;

    for (const auto& descriptor : board_manager.getEnumeratedList())
    {
        ifx_Radar_Sensor_List_Entry_t entry = {};

        try
        {
            std::unique_ptr<BoardInstance> board = descriptor->createBoardInstance();
            if (!get_sensor_type(board, entry.sensor_type))
            {
                // not a radar sensor that we support
                continue;
            }

            entry.board_type = rdk::RadarDeviceCommon::get_boardtype_from_pid(board->getPid());

            // read uuid
            const auto uuid = board->getUuidString();
            std::copy(uuid.begin(), uuid.end(), entry.uuid);

            if (selector(entry))
                list.push_back(entry);
        }
        catch (const EException&)
        {
            continue;
        }
    }

    return list;
}

}  // namespace


bool rdk::RadarDeviceCommon::get_sensor_type(std::unique_ptr<BoardInstance>& board, ifx_Radar_Sensor_t& sensor_type)
{
    const auto version = board->getIVendorCommands()->getProtocolVersion();
    if (version < 0x00030001)
    {
        return get_sensor_type_legacy(board, sensor_type);
    }

    try
    {
        const auto count = board->getComponentCount<IRadarAvian>();
        if (count)
        {
            sensor_type = get_avian_type(board);
            return true;
        }
    }
    catch (...)
    {}   

    try
    {
        const auto count = board->getComponentCount<IRadarAtr22>();
        if (count)
        {
            sensor_type = IFX_MIMOSE_BGT24ATR22;
            return true;
        }
    }
    catch (...)
    {}

    try
    {
        const auto count = board->getComponentCount<IRadarLtr11>();
        if (count)
        {
            sensor_type = IFX_BGT60LTR11AIP;
            return true;
        }
    }
    catch (...)
    {}

    return false;
}

std::unique_ptr<BoardInstance> rdk::RadarDeviceCommon::open(SelectorFunction&& selector)
{
    std::unique_lock<std::mutex> lock(mutex_board_manager);

    BoardManager board_manager;
    if (use_serial)
    {
        board_manager.useSerial();
    }
    if (use_ethernet)
    {
        board_manager.useUdp();
    }
    if (use_uvc)
    {
        board_manager.useUvc();
    }
    if (use_wiggler)
    {
        board_manager.useWiggler();
    }
    if (use_libusb)
    {
        board_manager.useLibusb();
    }
    board_manager.enumerate();

    auto list = ::get_list(board_manager, std::forward<SelectorFunction>(selector));
    if (list.empty())
        return nullptr;

    try
    {
        uint8_t uuid_array[16];
        if (ifx_uuid_from_string(list[0].uuid, uuid_array))
        {
            return board_manager.createSpecificBoardInstance(uuid_array);
        }
    }
    catch (EException&)
    {
    }

    return nullptr;
}

std::unique_ptr<BoardInstance> rdk::RadarDeviceCommon::open_by_port(const char port[])
{
    try
    {
        return BoardSerial::createBoardInstance(port);
    }
    catch (EException&)
    {
        return nullptr;
    }
}

std::unique_ptr<BoardInstance> rdk::RadarDeviceCommon::open_by_uuid(const char uuid[])
{
    uint8_t uuid_array[16];
    if (!ifx_uuid_from_string(uuid, uuid_array))
    {
        throw rdk::exception::argument_invalid();
    }

    std::unique_lock<std::mutex> lock(mutex_board_manager);

    BoardManager board_manager;
    if (use_serial)
    {
        board_manager.useSerial();
    }
    if (use_ethernet)
    {
        board_manager.useUdp();
    }
    if (use_uvc)
    {
        board_manager.useUvc();
    }
    if (use_wiggler)
    {
        board_manager.useWiggler();
    }
    if (use_libusb)
    {
        board_manager.useLibusb();
    }
    board_manager.enumerate();

    try
    {
        return board_manager.createSpecificBoardInstance(uuid_array);
    }
    catch (EException&)
    {
    }

    return nullptr;
}

std::vector<ifx_Radar_Sensor_List_Entry_t> rdk::RadarDeviceCommon::get_list(SelectorFunction&& selector)
{
    std::unique_lock<std::mutex> lock(mutex_board_manager);

    BoardManager board_manager;
    if (use_serial)
    {
        board_manager.useSerial();
    }
    if (use_ethernet)
    {
        board_manager.useUdp();
    }
    if (use_uvc)
    {
        board_manager.useUvc();
    }
    if (use_wiggler)
    {
        board_manager.useWiggler();
    }
    if (use_libusb)
    {
        board_manager.useLibusb();
    }
    board_manager.enumerate();

    return ::get_list(board_manager, std::forward<SelectorFunction>(selector));
}

void rdk::RadarDeviceCommon::get_firmware_info(BoardInstance* board, ifx_Firmware_Info_t* firmware_info)
{
    if (board)
    {
        const auto version = board->getIBridge()->getIBridgeControl()->getVersionInfo();
        firmware_info->description = board->getName();
        firmware_info->version_major = version[0];
        firmware_info->version_minor = version[1];
        firmware_info->version_build = version[2];
        firmware_info->extendedVersion = board->getIBridge()->getIBridgeControl()->getExtendedVersionString().c_str();
    }
    else
    {
        firmware_info->description = "";
        firmware_info->version_major = 0;
        firmware_info->version_minor = 0;
        firmware_info->version_build = 0;
        firmware_info->extendedVersion = "";
    }
}

bool rdk::RadarDeviceCommon::is_firmware_version_valid(const std::array<uint16_t, 3>& min_version, ifx_Firmware_Info_t firmware_info)
{
    const std::array<uint16_t, 3> version {firmware_info.version_major, firmware_info.version_minor, firmware_info.version_build};
    return !std::lexicographical_compare(version.begin(), version.end(), min_version.begin(), min_version.end());
}
