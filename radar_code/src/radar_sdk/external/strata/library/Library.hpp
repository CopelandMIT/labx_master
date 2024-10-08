/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "Definitions.hpp"
#include <cstdint>
#include <string>


namespace Strata
{
    /**
     * Get library version and build number of Strata.
     *
     * \param major Contains major version number on return
     * \param minor Contains minor version number on return
     * \param patch Contains patch number on return
     * \param build Contains build number on return
     */
    STRATA_API void getVersion(uint16_t &major, uint16_t &minor, uint16_t &patch, uint16_t &build);

    STRATA_API std::string getVersionString();
}
