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

#include <stdlib.h>
#include <string.h>

#include "ifxBase/Complex.h"
#include "ifxBase/Defines.h"
#include "ifxBase/Error.h"
#include "ifxBase/internal/Macros.h"
#include "ifxBase/Math.h"
#include "ifxBase/Mem.h"
#include "ifxBase/Vector.h"

#include "ifxRadar/AngleMonopulse.h"

/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

/**
 * @brief Defines the structure for Angle Monopulse module.
 *        Use type ifx_AngleMonopulse_t for this struct.
 */
struct ifx_AngleMonopulse_s
{
    ifx_Float_t wavelength;      /**< Wavelength (in units of meters), computed from center frequency and speed of light.*/
    ifx_Float_t antenna_spacing; /**< Physical spacing (in units of meters), between antennas on BGT radar chip.*/
};

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_AngleMonopulse_t* ifx_anglemonopulse_create(ifx_Float_t wavelength,
                                                ifx_Float_t antenna_spacing)
{
    IFX_ERR_BRN_COND(wavelength <= 0 || antenna_spacing <= 0,
                     IFX_ERROR_ARGUMENT_OUT_OF_BOUNDS);

    ifx_AngleMonopulse_t* h = ifx_mem_alloc(sizeof(struct ifx_AngleMonopulse_s));
    IFX_ERR_BRN_MEMALLOC(h);

    h->wavelength = wavelength;

    h->antenna_spacing = antenna_spacing;

    return h;
}

//----------------------------------------------------------------------------

void ifx_anglemonopulse_destroy(ifx_AngleMonopulse_t* handle)
{
    if (handle == NULL)
    {
        return;
    }

    ifx_mem_free(handle);
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_anglemonopulse_scalar_run(const ifx_AngleMonopulse_t* handle,
                                          ifx_Complex_t rx1,
                                          ifx_Complex_t rx2)
{
    IFX_ERR_BRV_NULL(handle, 0);

    ifx_Float_t d_phi;
    ifx_Float_t target_angle;
    ifx_Float_t wavelength_ant_spacing_ratio = (handle->wavelength / handle->antenna_spacing);

    const ifx_Float_t rx1_ang =
        ATAN2(IFX_COMPLEX_IMAG(rx1), IFX_COMPLEX_REAL(rx1));
    const ifx_Float_t rx2_ang =
        ATAN2(IFX_COMPLEX_IMAG(rx2), IFX_COMPLEX_REAL(rx2));

    d_phi = (rx1_ang - rx2_ang);

    if (d_phi <= -IFX_PI)
    {
        d_phi += 2 * IFX_PI;
    }
    else if (d_phi > IFX_PI)
    {
        d_phi -= 2 * IFX_PI;
    }

    /* Arcus sinus (-PI/2 to PI/2), input= -1..1 */
    target_angle = ASIN(d_phi * wavelength_ant_spacing_ratio / (2 * IFX_PI));

    return target_angle * (180 / IFX_PI);  // Angle (-90...90°)
}

//----------------------------------------------------------------------------

void ifx_anglemonopulse_vector_run(const ifx_AngleMonopulse_t* handle,
                                   const ifx_Vector_C_t* rx1,
                                   const ifx_Vector_C_t* rx2,
                                   ifx_Vector_R_t* target_angle_deg)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_VEC_BRK_VALID(rx1);
    IFX_VEC_BRK_VALID(rx2);
    IFX_VEC_BRK_VALID(target_angle_deg);

    IFX_VEC_BRK_DIM(rx1, rx2);
    IFX_VEC_BRK_DIM_GT(rx1, target_angle_deg);

    for (uint32_t i = 0; i < vLen(rx1); ++i)
    {
        vAt(target_angle_deg, i) =
            ifx_anglemonopulse_scalar_run(handle, vAt(rx1, i), vAt(rx2, i));
    }
}

//----------------------------------------------------------------------------

void ifx_anglemonopulse_set_wavelength(ifx_AngleMonopulse_t* handle,
                                       ifx_Float_t wavelength)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_NEGATIVE0(wavelength);

    handle->wavelength = wavelength;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_anglemonopulse_get_wavelength(const ifx_AngleMonopulse_t* handle)
{
    IFX_ERR_BRV_NULL(handle, 0);

    return handle->wavelength;
}

//----------------------------------------------------------------------------

void ifx_anglemonopulse_set_antenna_spacing(ifx_AngleMonopulse_t* handle,
                                            ifx_Float_t antenna_spacing)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_NEGATIVE0(antenna_spacing);

    handle->antenna_spacing = antenna_spacing;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_anglemonopulse_get_antenna_spacing(const ifx_AngleMonopulse_t* handle)
{
    IFX_ERR_BRV_NULL(handle, 0);

    return handle->antenna_spacing;
}
