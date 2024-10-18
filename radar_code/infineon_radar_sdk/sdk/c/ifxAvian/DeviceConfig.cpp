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

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/
#include "DeviceConfig.h"

#include "ifxBase/Error.h"
#include "ifxBase/internal/Util.h"

#include <cmath>

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

float ifx_devconf_get_center_frequency(const ifx_Avian_Config_t* config)
{
    IFX_ERR_BRV_NULL(config, NAN);
    return static_cast<float>(config->start_frequency_Hz + config->end_frequency_Hz) / 2;
}

//----------------------------------------------------------------------------

float ifx_devconf_get_bandwidth(const ifx_Avian_Config_t* config)
{
    IFX_ERR_BRV_NULL(config, NAN);
    if (config->start_frequency_Hz > config->end_frequency_Hz)
    {
        return static_cast<float>(config->start_frequency_Hz - config->end_frequency_Hz);
    }
    else
    {
        return static_cast<float>(config->end_frequency_Hz - config->start_frequency_Hz);
    }
}

//----------------------------------------------------------------------------

float ifx_devconf_get_chirp_time(const ifx_Avian_Config_t* config)
{
    IFX_ERR_BRV_NULL(config, NAN);
    IFX_ERR_BRV_COND(config->num_samples_per_chirp == 0, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS, NAN);
    IFX_ERR_BRV_COND(config->sample_rate_Hz == 0, IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS, NAN);

    return (static_cast<float>(config->num_samples_per_chirp) / config->sample_rate_Hz);
}

//----------------------------------------------------------------------------

uint8_t ifx_devconf_count_rx_antennas(const ifx_Avian_Config_t* config)
{
    IFX_ERR_BRV_NULL(config, 0);

    const uint8_t tx_antennas = config->mimo_mode == IFX_MIMO_TDM ? 2 : 1;
    return uint8_t(ifx_util_popcount(config->rx_mask)) * tx_antennas;  // multiplying with tx antenna count to get virtual antenna count
}
