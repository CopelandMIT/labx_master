
#pragma once

// dshow.h needs to be included first
#include <dshow.h>

#include <comdef.h>
#include <devicetopology.h>


_COM_SMARTPTR_TYPEDEF(IGraphBuilder, IID_IGraphBuilder);
_COM_SMARTPTR_TYPEDEF(IMemAllocator, IID_IMemAllocator);
_COM_SMARTPTR_TYPEDEF(IMediaControl, IID_IMediaControl);
_COM_SMARTPTR_TYPEDEF(IBaseFilter, IID_IBaseFilter);
_COM_SMARTPTR_TYPEDEF(IMediaEventEx, IID_IMediaEventEx);

_COM_SMARTPTR_TYPEDEF(IKsControl, IID_IKsControl);


#ifndef _MSC_VER
_COM_SMARTPTR_TYPEDEF(IMoniker, IID_IMoniker);
#endif
