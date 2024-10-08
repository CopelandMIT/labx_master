/* ===========================================================================
** Copyright (C) 2021 Infineon Technologies AG
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

#pragma once

#include <ifxBase/Error.h>
#include <ifxBase/Exception.hpp>
#include <ifxBase/FunctionWrapper.hpp>
#include <ifxBase/Types.h>
#include <ifxRadarDeviceCommon/RadarDeviceCommon.h>

#include <platform/BoardInstance.hpp>
#include <platform/serial/BoardSerial.hpp>

#include <functional>

namespace rdk {
namespace RadarDeviceCommon {

/** Return true if sensor belongs to Avian family */
inline bool sensor_is_avian(ifx_Radar_Sensor_t sensor_type)
{
    return sensor_type <= IFX_AVIAN_UNKNOWN;
}

/** Return true if sensor belongs to Mimose family */
inline bool sensor_is_mimose(ifx_Radar_Sensor_t sensor_type)
{
    return sensor_type == IFX_MIMOSE_BGT24ATR22;
}

/** Return true if sensor is BGT60LTR11 */
inline bool sensor_is_ltr11(ifx_Radar_Sensor_t sensor_type)
{
    return sensor_type == IFX_BGT60LTR11AIP;
}

/** Return true if sensor is smartar*/
inline bool sensor_is_smartar(ifx_Radar_Sensor_t sensor_type)
{
   return false;
}

/**
 * @brief Determine the radar sensor
 *
 * Find out to which radar sensor we are currently connected.
 *
 * @param [in]  board           board instance
 * @param [out] sensor_type     type of radar sensor
 * @retval  true                if successful
 * @retval  false               if unknown (i.e., unsupported) radar sensor
 */
IFX_DLL_PUBLIC bool get_sensor_type(std::unique_ptr<BoardInstance>& board, ifx_Radar_Sensor_t& sensor_type);

/**
 * @brief Type of selector function
 *
 * If the selector function returns true the respective element will
 * be selected.
 */
using SelectorFunction = std::function<bool(const ifx_Radar_Sensor_List_Entry_t&)>;

/**
 * @brief Determine board type from product id
 *
 * @param [in]    pid            product id
 * @retval        boardtype    board type
 */
inline ifx_Board_Type_t get_boardtype_from_pid(uint16_t pid)
{
    constexpr uint16_t pid_RadarBaseboardMCU7 = 0x0251;   // PID of RadarBaseboardMCU7 (as returned by Strata)
    constexpr uint16_t pid_RadarBaseboardAurix = 0x0252;  // PID of RadarBaseboardAurix (as returned by Strata)
    constexpr uint16_t pid_V9 = 0x0253;                   // PID of V9 board (as returned by Strata)

    switch (pid)
    {
        case pid_RadarBaseboardAurix:
            return IFX_BOARD_BASEBOARD_AURIX;
        case pid_RadarBaseboardMCU7:
            return IFX_BOARD_BASEBOARD_MCU7;
        case pid_V9:
            return IFX_BOARD_BASEBOARD_V9;
        default:
            return IFX_BOARD_UNKNOWN;
    }
}

inline const char* sensor_to_string(ifx_Radar_Sensor_t sensor_type)
{
    switch (sensor_type)
    {
        case IFX_AVIAN_BGT60TR13C:
            return "BGT60TR13C";
        case IFX_AVIAN_BGT60ATR24C:
            return "BGT60ATR24C";
        case IFX_AVIAN_BGT60UTR13D:
            return "BGT60UTR13D";
        case IFX_AVIAN_BGT60UTR11AIP:
            return "BGT60UTR11AIP";
        case IFX_AVIAN_UNKNOWN:
            return "Unknown Avian sensor";

        case IFX_MIMOSE_BGT24ATR22:
            return "BGT24ATR22";

        case IFX_BGT60LTR11AIP:
            return "BGT60LTR11";

        default:
            return "unknown";
    }
}

inline ifx_Radar_Sensor_t string_to_sensor(const std::string& sensor)
{
    if (sensor == "BGT60TR13C")
        return IFX_AVIAN_BGT60TR13C;
    else if (sensor == "BGT60ATR24C")
        return IFX_AVIAN_BGT60ATR24C;
    else if (sensor == "BGT60UTR13D")
        return IFX_AVIAN_BGT60UTR13D;
    else if (sensor == "BGT60UTR11AIP")
        return IFX_AVIAN_BGT60UTR11AIP;

    else if (sensor == "BGT24ATR22")
        return IFX_MIMOSE_BGT24ATR22;

    else if (sensor == "BGT60LTR11")
        return IFX_BGT60LTR11AIP;
    else
        return IFX_RADAR_SENSOR_UNKNOWN;
}

inline const char* board_type_to_string(ifx_Board_Type_t board_type)
{
    switch (board_type)
    {
        case IFX_BOARD_BASEBOARD_AURIX:
            return "RadarBaseboardAurix";
        case IFX_BOARD_BASEBOARD_MCU7:
            return "RadarBaseboardMCU7";
        case IFX_BOARD_BASEBOARD_V9:
            return "V9";
        default:
            return "unknown";
    }
}

/**
 * @brief Open first board found
 *
 * Opens the first board found for which the selector function returns true.
 * If not selector is given, the first board found is opened.
 *
 * @param [in]    selector    selector function
 */
IFX_DLL_PUBLIC std::unique_ptr<BoardInstance> open(SelectorFunction&& selector = [](const ifx_Radar_Sensor_List_Entry_t&) { return true; });

/**
 * @brief Open board by port
 *
 * Open a board by port and return the Strata BoardInstance. If no
 * board with that port was found nullptr is returned.
 *
 * @param [in]    port        port as char string
 * @retval        board       board instance
 */
IFX_DLL_PUBLIC std::unique_ptr<BoardInstance> open_by_port(const char port[]);

/**
 * @brief Open board by unique id
 *
 * Open a board by UUID and return the Strata BoardInstance. If no
 * board with that uuid was found, nullptr is returned.
 *
 * In its canonical textual representation, the 16 octets of a UUID are represented
 * as 32 hexadecimal (base-16) digits, displayed in 5 groups separated by hyphens,
 * in the form 8-4-4-4-12 for a total of 36 characters (32 hexadecimal characters
 * and 4 hyphens). For example:
 *    123e4567-e89b-12d3-a456-426655440000
 *    xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx
 *
 * @param [in]    uuid        uuid as char string
 * @retval        board       board instance
 */
IFX_DLL_PUBLIC std::unique_ptr<BoardInstance> open_by_uuid(const char uuid[]);

/**
 * @brief Returns list of boards
 *
 * Returns a vector with all boards connected where the selector
 * function returns true.
 *
 * @param [in]    selector    selector function
 */
IFX_DLL_PUBLIC std::vector<ifx_Radar_Sensor_List_Entry_t> get_list(SelectorFunction&& selector);

/**
 * @brief Return firmware info
 *
 * The returned firmware info is valid until the board instance is
 * valid. If the board instance is destroyed the returned firmware
 * info must no longer be accessed.
 *
 * @param [in]    board     board instance
 * @retval        info      firmware info
 */
IFX_DLL_PUBLIC void get_firmware_info(BoardInstance* board, ifx_Firmware_Info_t* firmware_info);

/**
 * @brief checks whether the firmware is valid
 *
 * firmware info is compared to the minimum firmware requirement
 * returns true if firmware is valid otherwise false
 *
 * @param [in]    min_version   minimum required version
 * @param [in]    firmware_info firmware info
 * @retval        bool          is valid
 */
IFX_DLL_PUBLIC bool is_firmware_version_valid(const std::array<uint16_t, 3>& min_version, ifx_Firmware_Info_t firmware_info);


template <typename Device, typename... Args>
Device* open_device(Args&&... args)
{
    auto caller = [](Args&&... args) {
        return new Device(std::forward<Args>(args)...);
    };
    return call_func(caller, nullptr, std::forward<Args>(args)...);
}

template <typename Device, typename... Args>
Device* open_board(std::unique_ptr<BoardInstance>&& board, Args&&... args)
{
    auto caller = [&board](Args&&... args) {
        return new Device(std::move(board), std::forward<Args>(args)...);
    };
    return call_func(caller, nullptr, std::forward<Args>(args)...);
}

template <typename Device>
Device* open_board_by_uuid(const char* uuid)
{
    if (!uuid)
    {
        throw rdk::exception::argument_null();
    }
    auto board = open_by_uuid(uuid);
    return open_board<Device>(std::move(board));
}

template <typename Device>
Device* open_board_by_selector(SelectorFunction&& selector)
{
    auto board = open(std::forward<SelectorFunction>(selector));
    return open_board<Device>(std::move(board));
}

template <typename Device>
Device* open_board_by_port(const char* port)
{
    if (!port)
    {
        throw rdk::exception::argument_null();
    }
    auto board = open_by_port(port);
    return open_board<Device>(std::move(board));
}

}  // namespace RadarDeviceCommon
}  // namespace rdk
