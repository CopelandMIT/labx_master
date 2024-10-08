/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "Library.hpp"


std::string Strata::getVersionString()
{
    uint16_t m_versionInfo[4];
    getVersion(m_versionInfo[0], m_versionInfo[1], m_versionInfo[2], m_versionInfo[3]);

    std::string versionString = std::to_string(m_versionInfo[0]) + '.' +
                                std::to_string(m_versionInfo[1]) + '.' +
                                std::to_string(m_versionInfo[2]);
    if (m_versionInfo[3])
    {
        versionString += '+';
        versionString += std::to_string(m_versionInfo[3]);
    }

    return versionString;
}
