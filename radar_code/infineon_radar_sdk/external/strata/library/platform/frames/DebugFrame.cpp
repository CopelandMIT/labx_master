/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "DebugFrame.hpp"

#include <common/Logger.hpp>


void DebugFrame::log(uint8_t *payload, uint32_t length, uint64_t timestamp)
{
    auto message = reinterpret_cast<char *>(payload);
    auto log     = LOG(DEBUG);
    log << "[REMOTE] " << timestamp / 1000 << " : ";
    log.write(message, length);
}
