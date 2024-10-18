/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

// dshow.h needs to be included first
#include <dshow.h>

#include <comdef.h>


// define COM smart pointers
_COM_SMARTPTR_TYPEDEF(ICreateDevEnum, (IID_ICreateDevEnum));

#ifndef _MSC_VER
_COM_SMARTPTR_TYPEDEF(IEnumMoniker, IID_IEnumMoniker);
_COM_SMARTPTR_TYPEDEF(IPropertyBag, IID_IPropertyBag);
#endif
