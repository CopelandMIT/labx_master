#pragma once


#ifdef _MSC_VER

    #include <vidcap.h>

    #define IID_IKsTopologyInfo __uuidof(IKsTopologyInfo)

#else

    #include <comdef.h>
    #include <ks.h>

MIDL_INTERFACE("720D4AC0-7533-11D0-A5D6-28DB04C10000")
IKsTopologyInfo :
    public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE get_NumCategories(
        /* [out] */ DWORD * pdwNumCategories) = 0;

    virtual HRESULT STDMETHODCALLTYPE get_Category(
        /* [in] */ DWORD dwIndex,
        /* [out] */ GUID * pCategory) = 0;

    virtual HRESULT STDMETHODCALLTYPE get_NumConnections(
        /* [out] */ DWORD * pdwNumConnections) = 0;

    virtual HRESULT STDMETHODCALLTYPE get_ConnectionInfo(
        /* [in] */ DWORD dwIndex,
        /* [out] */ KSTOPOLOGY_CONNECTION * pConnectionInfo) = 0;

    virtual HRESULT STDMETHODCALLTYPE get_NodeName(
        /* [in] */ DWORD dwNodeId,
        /* [annotation][out] */
        //__out_bcount_opt(dwBufSize)
        WCHAR * pwchNodeName,
        /* [in] */ DWORD dwBufSize,
        /* [annotation][out] */
        //__out
        DWORD * pdwNameLen) = 0;

    virtual HRESULT STDMETHODCALLTYPE get_NumNodes(
        /* [out] */ DWORD * pdwNumNodes) = 0;

    virtual HRESULT STDMETHODCALLTYPE get_NodeType(
        /* [in] */ DWORD dwNodeId,
        /* [out] */ GUID * pNodeType) = 0;

    virtual HRESULT STDMETHODCALLTYPE CreateNodeInstance(
        /* [in] */ DWORD dwNodeId,
        /* [in] */ REFIID iid,
        /* [out] */ void **ppvObject) = 0;
};


    #include <initguid.h>

DEFINE_GUID(IID_IKsTopologyInfo, 0x720D4AC0, 0x7533, 0x11D0, 0xA5, 0xD6, 0x28, 0xDB, 0x04, 0xC1, 0x00, 0x00);
DEFINE_GUID(KSNODETYPE_DEV_SPECIFIC, 0x941c7ac0, 0xc559, 0x11d0, 0x8a, 0x2b, 0x00, 0xa0, 0xc9, 0x25, 0x5a, 0xc1);


#endif
