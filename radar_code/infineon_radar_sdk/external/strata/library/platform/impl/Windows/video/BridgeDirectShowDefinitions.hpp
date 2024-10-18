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

#include "VidCap.hpp"
#include <comdef.h>
#include <strmif.h>


EXTERN_C const CLSID CLSID_SampleGrabber;
EXTERN_C const CLSID CLSID_NullRenderer;


_COM_SMARTPTR_TYPEDEF(ICaptureGraphBuilder2, IID_ICaptureGraphBuilder2);
_COM_SMARTPTR_TYPEDEF(IAMStreamConfig, IID_IAMStreamConfig);
_COM_SMARTPTR_TYPEDEF(IMediaFilter, IID_IMediaFilter);
_COM_SMARTPTR_TYPEDEF(IPin, IID_IPin);
_COM_SMARTPTR_TYPEDEF(IMemInputPin, IID_IMemInputPin);
_COM_SMARTPTR_TYPEDEF(IKsTopologyInfo, IID_IKsTopologyInfo);
