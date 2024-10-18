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

#include "Complex.h"
#include "Defines.h"

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

/*
==============================================================================
   4. LOCAL DATA
==============================================================================
*/

/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_Float_t ifx_complex_abs(ifx_Complex_t z)
{
    ifx_Float_t r = IFX_COMPLEX_REAL(z);
    ifx_Float_t i = IFX_COMPLEX_IMAG(z);

    return HYPOT(r, i);
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_complex_conj(ifx_Complex_t z)
{
    ifx_Complex_t conj;
    IFX_COMPLEX_SET(conj, IFX_COMPLEX_REAL(z), -IFX_COMPLEX_IMAG(z));
    return conj;
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_complex_add(ifx_Complex_t a,
                              ifx_Complex_t b)
{
    ifx_Complex_t result = IFX_COMPLEX_DEF(1, -1);

    IFX_COMPLEX_SET(result,
                    IFX_COMPLEX_REAL(a) + IFX_COMPLEX_REAL(b),
                    IFX_COMPLEX_IMAG(a) + IFX_COMPLEX_IMAG(b));

    return result;
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_complex_sub(ifx_Complex_t a,
                              ifx_Complex_t b)
{
    ifx_Complex_t result;
    IFX_COMPLEX_SET(result,
                    IFX_COMPLEX_REAL(a) - IFX_COMPLEX_REAL(b),
                    IFX_COMPLEX_IMAG(a) - IFX_COMPLEX_IMAG(b));

    return result;
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_complex_mul(ifx_Complex_t a,
                              ifx_Complex_t b)
{
    ifx_Complex_t result;
    ifx_Float_t ar = IFX_COMPLEX_REAL(a);
    ifx_Float_t ai = IFX_COMPLEX_IMAG(a);
    ifx_Float_t br = IFX_COMPLEX_REAL(b);
    ifx_Float_t bi = IFX_COMPLEX_IMAG(b);

    IFX_COMPLEX_SET(result,
                    ar * br - ai * bi,
                    ar * bi + ai * br);

    return result;
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_complex_div(ifx_Complex_t a,
                              ifx_Complex_t b)
{
    const ifx_Complex_t conjb = ifx_complex_conj(b);

    const ifx_Float_t r = IFX_COMPLEX_REAL(b);
    const ifx_Float_t i = IFX_COMPLEX_IMAG(b);
    const ifx_Float_t b_abs2 = r * r + i * i;  // |b|^2

    // a/b = (ab*) / (bb*) = (ab*)/(|b|^2)
    return ifx_complex_div_real(ifx_complex_mul(a, conjb), b_abs2);
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_complex_add_real(ifx_Complex_t a,
                                   ifx_Float_t b)
{
    ifx_Complex_t result;
    IFX_COMPLEX_SET(result, IFX_COMPLEX_REAL(a) + b, IFX_COMPLEX_IMAG(a));
    return result;
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_complex_sub_real(ifx_Complex_t a,
                                   ifx_Float_t b)
{
    ifx_Complex_t result;
    IFX_COMPLEX_SET(result, IFX_COMPLEX_REAL(a) - b, IFX_COMPLEX_IMAG(a));
    return result;
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_complex_mul_real(ifx_Complex_t a,
                                   ifx_Float_t b)
{
    ifx_Complex_t result;
    IFX_COMPLEX_SET(result, IFX_COMPLEX_REAL(a) * b, IFX_COMPLEX_IMAG(a) * b);
    return result;
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_complex_div_real(ifx_Complex_t a,
                                   ifx_Float_t b)
{
    ifx_Complex_t result;
    IFX_COMPLEX_SET(result, IFX_COMPLEX_REAL(a) / b, IFX_COMPLEX_IMAG(a) / b);
    return result;
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_complex_ln(ifx_Complex_t z)
{
    ifx_Complex_t result;
    ifx_Polar_t zp = ifx_complex_to_polar(z);

    IFX_COMPLEX_SET(result, LOGN(zp.radius), zp.angle);

    return result;
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_complex_log10(ifx_Complex_t z)
{
    return ifx_complex_mul_real(ifx_complex_ln(z), 1 / LOGN(10.0));
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_complex_arg(ifx_Complex_t z)
{
    ifx_Float_t i = IFX_COMPLEX_IMAG(z);
    ifx_Float_t r = IFX_COMPLEX_REAL(z);

    if (i == 0 && r == 0)
    {
        return 0;
    }

    return ATAN2(i, r);
}

//----------------------------------------------------------------------------

ifx_Polar_t ifx_complex_to_polar(ifx_Complex_t z)
{
    ifx_Polar_t polar = {0};

    polar.radius = ifx_complex_abs(z);
    polar.angle = ifx_complex_arg(z);

    return polar;
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_complex_from_polar(ifx_Polar_t zp)
{
    ifx_Complex_t z = IFX_COMPLEX_DEF(zp.radius * COS(zp.angle), zp.radius * SIN(zp.angle));
    return z;
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_complex_square(ifx_Complex_t z)
{
    ifx_Complex_t result;

    ifx_Float_t r = IFX_COMPLEX_REAL(z);
    ifx_Float_t i = IFX_COMPLEX_IMAG(z);

    IFX_COMPLEX_SET(result, (r * r - i * i), 2 * r * i);

    return result;
}

//----------------------------------------------------------------------------

ifx_Float_t ifx_complex_sqnorm(ifx_Complex_t z)
{
    ifx_Float_t r = IFX_COMPLEX_REAL(z);
    ifx_Float_t i = IFX_COMPLEX_IMAG(z);

    return (r * r + i * i);
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_complex_sqrt(ifx_Complex_t z)
{
    ifx_Complex_t result;

    ifx_Float_t r = IFX_COMPLEX_REAL(z);
    ifx_Float_t i = IFX_COMPLEX_IMAG(z);

    ifx_Float_t z_abs = ifx_complex_abs(z);

    IFX_COMPLEX_SET(result, SQRT((z_abs + r) / 2), COPYSIGN(1.0, i) * SQRT((z_abs - r) / 2));

    return result;
}

//----------------------------------------------------------------------------

ifx_Complex_t ifx_complex_pow(ifx_Complex_t z, uint16_t n)
{
    ifx_Complex_t result_c;

    if (n == 0)
    {
        IFX_COMPLEX_SET(result_c, 1, 0);
        return (result_c);
    }

    if (n == 1)
    {
        return (z);
    }

    ifx_Float_t abs_z = ifx_complex_abs(z);

    if (abs_z == 0.0)
    {
        IFX_COMPLEX_SET(result_c, 1, 0);
        return (result_c);
    }

    ifx_Float_t arg_z = ifx_complex_arg(z);
    ifx_Float_t r = POW(abs_z, n);
    ifx_Float_t theta = n * arg_z;

    IFX_COMPLEX_SET(result_c, r * COS(theta), r * SIN(theta));
    return result_c;
}
