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

/**
 * @file Complex.h
 *
 * \brief \copybrief gr_complex
 *
 * For details refer to \ref gr_complex
 */

#ifndef IFX_BASE_COMPLEX_H
#define IFX_BASE_COMPLEX_H

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "Defines.h"
#include "Types.h"


#ifdef __cplusplus
extern "C"
{
#endif


/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

/*
==============================================================================
   3. TYPES
==============================================================================
*/

#define IFX_COMPLEX_REAL(c) ((c).data[0])
#define IFX_COMPLEX_IMAG(c) ((c).data[1])
#define IFX_COMPLEX_SET_REAL(c, r) \
    do                             \
    {                              \
        (c).data[0] = r;           \
    } while (0)
#define IFX_COMPLEX_SET_IMAG(c, i) \
    do                             \
    {                              \
        (c).data[1] = i;           \
    } while (0)
#define IFX_COMPLEX_DEF(r, i)                  \
    {                                          \
        {                                      \
            (ifx_Float_t)(r), (ifx_Float_t)(i) \
        }                                      \
    }
#define IFX_COMPLEX_SET(c, r, i)    \
    do                              \
    {                               \
        IFX_COMPLEX_SET_REAL(c, r); \
        IFX_COMPLEX_SET_IMAG(c, i); \
    } while (0)

#define IFX_COMPLEX_IS_EQUAL(a, b)                \
    ((IFX_COMPLEX_REAL(a) == IFX_COMPLEX_REAL(b)) \
     && (IFX_COMPLEX_IMAG(a) == IFX_COMPLEX_IMAG(b)))

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/** @addtogroup gr_cat_SDK_base
 * @{
 */

/** @defgroup gr_complex Complex
 * @brief API for operations on complex numbers
 *
 * @{
 */

/**
 * @brief Computes the absolute value of a complex number |z|.
 *
 * @param [in]     z         Complex number defined by \ref ifx_Complex_t.
 *
 * @return Computed absolute value: square root of sum of squares (hypotenuse) defined by \see ifx_Float_t.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_complex_abs(ifx_Complex_t z);

/**
 * @brief Computes the complex conjugate of a complex number.
 *
 * @param [in]     z         Complex number defined by \ref ifx_Complex_t.
 *
 * @return Computed complex conjugate defined by \ref ifx_Complex_t.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_complex_conj(ifx_Complex_t z);

/**
 * @brief Adds two complex numbers a + b.
 *
 * @param [in]     a         Complex number defined by \ref ifx_Complex_t.
 * @param [in]     b         Complex number defined by \ref ifx_Complex_t.
 *
 * @return Addition result of the passed complex numbers defined by \ref ifx_Complex_t.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_complex_add(ifx_Complex_t a,
                              ifx_Complex_t b);

/**
 * @brief Subtracts two complex numbers a - b.
 *
 * @param [in]     a         Complex number defined by \ref ifx_Complex_t.
 * @param [in]     b         Complex number defined by \ref ifx_Complex_t.
 *
 * @return Subtraction result of the passed complex numbers defined by \ref ifx_Complex_t.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_complex_sub(ifx_Complex_t a,
                              ifx_Complex_t b);

/**
 * @brief Applies multiplication of two complex numbers a * b.
 *
 * @param [in]     a         Complex number defined by \ref ifx_Complex_t.
 * @param [in]     b         Complex number defined by \ref ifx_Complex_t.
 *
 * @return Multiplication result of the passed complex numbers defined by \ref ifx_Complex_t.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_complex_mul(ifx_Complex_t a,
                              ifx_Complex_t b);

/**
 * @brief Applies division of two complex numbers a / b.
 *
 * @param [in]     a         Complex number defined by \ref ifx_Complex_t.
 * @param [in]     b         Complex number defined by \ref ifx_Complex_t.
 *
 * @return Division result of the passed complex numbers defined by \ref ifx_Complex_t.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_complex_div(ifx_Complex_t a,
                              ifx_Complex_t b);

/**
 * @brief Adds a real value to a complex number.
 *
 * @param [in]     a         Complex number defined by \ref ifx_Complex_t.
 * @param [in]     b         Real floating value defined by \see ifx_Float_t.
 *
 * @return Addition result of the passed complex number and real value defined by \ref ifx_Complex_t.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_complex_add_real(ifx_Complex_t a,
                                   ifx_Float_t b);

/**
 * @brief Subtracts a real value from a complex number.
 *
 * @param [in]     a         Complex number defined by \ref ifx_Complex_t.
 * @param [in]     b         Real floating value defined by \see ifx_Float_t.
 *
 * @return Subtraction result of the passed complex number and real value defined by \ref ifx_Complex_t.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_complex_sub_real(ifx_Complex_t a,
                                   ifx_Float_t b);

/**
 * @brief Applies multiplication by real value to a complex number.
 *
 * @param [in]     a         Complex number defined by \ref ifx_Complex_t.
 * @param [in]     b         Real floating value defined by \see ifx_Float_t.
 *
 * @return Multiplication result of the passed complex number by a real value defined by \ref ifx_Complex_t.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_complex_mul_real(ifx_Complex_t a,
                                   ifx_Float_t b);

/**
 * @brief Applies division by real value to a complex number.
 *
 * @param [in]     a         Complex number defined by \ref ifx_Complex_t.
 * @param [in]     b         Real floating point values defined by \see ifx_Float_t.
 *
 * @return Division result of the passed complex number by a real value defined by \ref ifx_Complex_t.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_complex_div_real(ifx_Complex_t a,
                                   ifx_Float_t b);

/**
 * @brief Computes the complex logarithm of a complex number \f$\log{z}\f$.
 *
 * The function returns the principal value of the complex natural logarithm
 * such that \f$\mathrm{Im}\left(\log z\right) \in (-\pi, \pi)\f$.
 *
 * @param [in]     z         Complex number defined by \ref ifx_Complex_t.
 *
 * @return Computed complex logarithm the passed complex number defined by \ref ifx_Complex_t.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_complex_ln(ifx_Complex_t z);

/**
 * @brief Computes the complex logarithm to the base 10 of a complex number log10(z).
 *
 * @param [in]     z         Complex number defined by \ref ifx_Complex_t.
 *
 * @return Computed complex log10 of the passed complex number defined by \ref ifx_Complex_t.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_complex_log10(ifx_Complex_t z);

/**
 * @brief Computes the complex argument of a complex number.
 *
 * For a complex number \f$z=r e^{i\varphi}\f$ the function returns
 * the argument \f$\varphi\f$. The argument \f$\varphi\f$ is between \f$-\pi\f$
 * and \f$\pi\f$.
 *
 * @param [in]     z         Complex number defined by \ref ifx_Complex_t.
 *
 * @return Computed complex argument
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_complex_arg(ifx_Complex_t z);

/**
 * @brief Returns the polar form of a complex number.
 *
 * @param [in]     z         Complex number defined by \ref ifx_Complex_t.
 *
 * @return Polar form of the of the passed complex number defined by \ref ifx_Polar_t.
 *
 */
IFX_DLL_PUBLIC
ifx_Polar_t ifx_complex_to_polar(ifx_Complex_t z);

/**
 * @brief Returns the complex number of a polar form.
 *
 * @param [in]     zp         Polar form of a complex number defined by \ref ifx_Polar_t.
 *
 * @return Complex number of the passed polar form defined by \ref ifx_Complex_t.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_complex_from_polar(ifx_Polar_t zp);

/**
 * @brief Computes the square value of a complex number (a + bi)².
 *
 * @param [in]     z         Complex number defined by \ref ifx_Complex_t.
 *
 * @return Computed square value of the passed complex number.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_complex_square(ifx_Complex_t z);

/**
 * @brief Computes the squared norm (absolute square) of a complex number |z|².
 *
 * @param [in]     z         Complex number defined by \ref ifx_Complex_t.
 *
 * @return Computed squared norm value of the passed complex number defined by \see ifx_Float_t.
 *
 */
IFX_DLL_PUBLIC
ifx_Float_t ifx_complex_sqnorm(ifx_Complex_t z);

/**
 * @brief Computes the square root of a complex number z.
 *
 * @param [in]     z         Complex number defined by \ref ifx_Complex_t.
 *
 * @return Computed square root value of the passed complex number defined by \see ifx_Float_t.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_complex_sqrt(ifx_Complex_t z);

/**
 * @brief Computes the nth power of a complex number z.
 *
 * @param [in]     z         Complex number defined by \ref ifx_Complex_t.
 * @param [in]     n         Unsigned integer
 *
 * @return Computed the nth power of the passed complex number.
 *
 */
IFX_DLL_PUBLIC
ifx_Complex_t ifx_complex_pow(ifx_Complex_t z, uint16_t n);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* IFX_BASE_COMPLEX_H */
