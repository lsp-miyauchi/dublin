

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 12:14:07 2038
 */
/* Compiler settings for lspexif.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0628 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __lspexif_h__
#define __lspexif_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if defined(_CONTROL_FLOW_GUARD_XFG)
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef __IExifParser_FWD_DEFINED__
#define __IExifParser_FWD_DEFINED__
typedef interface IExifParser IExifParser;

#endif 	/* __IExifParser_FWD_DEFINED__ */


#ifndef __IRawThmubnail_FWD_DEFINED__
#define __IRawThmubnail_FWD_DEFINED__
typedef interface IRawThmubnail IRawThmubnail;

#endif 	/* __IRawThmubnail_FWD_DEFINED__ */


#ifndef __ExifParser_FWD_DEFINED__
#define __ExifParser_FWD_DEFINED__

#ifdef __cplusplus
typedef class ExifParser ExifParser;
#else
typedef struct ExifParser ExifParser;
#endif /* __cplusplus */

#endif 	/* __ExifParser_FWD_DEFINED__ */


#ifndef __RawThmubnail_FWD_DEFINED__
#define __RawThmubnail_FWD_DEFINED__

#ifdef __cplusplus
typedef class RawThmubnail RawThmubnail;
#else
typedef struct RawThmubnail RawThmubnail;
#endif /* __cplusplus */

#endif 	/* __RawThmubnail_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IExifParser_INTERFACE_DEFINED__
#define __IExifParser_INTERFACE_DEFINED__

/* interface IExifParser */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IExifParser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("53C45C07-F5F1-4773-9700-0FA3BA8D3499")
    IExifParser : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsExifFile( 
            /* [in] */ LPCWSTR pszFileName) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetExifInfo( 
            /* [in] */ LPCWSTR pszFileName,
            IStream *pStream,
            int *pnOrientation,
            BOOL bLargeImage,
            BYTE *pInfo) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetThumbsnail( 
            /* [in] */ LPCWSTR pszFileName,
            IStream *pStream,
            int *pnOrientation,
            BOOL bLargeImage,
            LPWSTR pszTips,
            int cchMax) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetInfoTip( 
            /* [in] */ LPCWSTR pszFileName,
            LPWSTR pszTips,
            int cchMax) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDateTimeString( 
            /* [in] */ LPCWSTR pszFileName,
            LPWSTR pszDate,
            int cchMax) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EnumExifInfo( 
            /* [in] */ LPCWSTR pszFileName,
            UINT_PTR pfnCallback,
            LPARAM param) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IExifParserVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IExifParser * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IExifParser * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IExifParser * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IExifParser * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IExifParser * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IExifParser * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IExifParser * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IExifParser, IsExifFile)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsExifFile )( 
            IExifParser * This,
            /* [in] */ LPCWSTR pszFileName);
        
        DECLSPEC_XFGVIRT(IExifParser, GetExifInfo)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetExifInfo )( 
            IExifParser * This,
            /* [in] */ LPCWSTR pszFileName,
            IStream *pStream,
            int *pnOrientation,
            BOOL bLargeImage,
            BYTE *pInfo);
        
        DECLSPEC_XFGVIRT(IExifParser, GetThumbsnail)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetThumbsnail )( 
            IExifParser * This,
            /* [in] */ LPCWSTR pszFileName,
            IStream *pStream,
            int *pnOrientation,
            BOOL bLargeImage,
            LPWSTR pszTips,
            int cchMax);
        
        DECLSPEC_XFGVIRT(IExifParser, GetInfoTip)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetInfoTip )( 
            IExifParser * This,
            /* [in] */ LPCWSTR pszFileName,
            LPWSTR pszTips,
            int cchMax);
        
        DECLSPEC_XFGVIRT(IExifParser, GetDateTimeString)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDateTimeString )( 
            IExifParser * This,
            /* [in] */ LPCWSTR pszFileName,
            LPWSTR pszDate,
            int cchMax);
        
        DECLSPEC_XFGVIRT(IExifParser, EnumExifInfo)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EnumExifInfo )( 
            IExifParser * This,
            /* [in] */ LPCWSTR pszFileName,
            UINT_PTR pfnCallback,
            LPARAM param);
        
        END_INTERFACE
    } IExifParserVtbl;

    interface IExifParser
    {
        CONST_VTBL struct IExifParserVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IExifParser_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IExifParser_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IExifParser_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IExifParser_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IExifParser_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IExifParser_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IExifParser_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IExifParser_IsExifFile(This,pszFileName)	\
    ( (This)->lpVtbl -> IsExifFile(This,pszFileName) ) 

#define IExifParser_GetExifInfo(This,pszFileName,pStream,pnOrientation,bLargeImage,pInfo)	\
    ( (This)->lpVtbl -> GetExifInfo(This,pszFileName,pStream,pnOrientation,bLargeImage,pInfo) ) 

#define IExifParser_GetThumbsnail(This,pszFileName,pStream,pnOrientation,bLargeImage,pszTips,cchMax)	\
    ( (This)->lpVtbl -> GetThumbsnail(This,pszFileName,pStream,pnOrientation,bLargeImage,pszTips,cchMax) ) 

#define IExifParser_GetInfoTip(This,pszFileName,pszTips,cchMax)	\
    ( (This)->lpVtbl -> GetInfoTip(This,pszFileName,pszTips,cchMax) ) 

#define IExifParser_GetDateTimeString(This,pszFileName,pszDate,cchMax)	\
    ( (This)->lpVtbl -> GetDateTimeString(This,pszFileName,pszDate,cchMax) ) 

#define IExifParser_EnumExifInfo(This,pszFileName,pfnCallback,param)	\
    ( (This)->lpVtbl -> EnumExifInfo(This,pszFileName,pfnCallback,param) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IExifParser_INTERFACE_DEFINED__ */


#ifndef __IRawThmubnail_INTERFACE_DEFINED__
#define __IRawThmubnail_INTERFACE_DEFINED__

/* interface IRawThmubnail */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IRawThmubnail;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("057A7B27-9C4D-4C48-82BF-8F40F04F1FA7")
    IRawThmubnail : public IDispatch
    {
    public:
    };
    
    
#else 	/* C style interface */

    typedef struct IRawThmubnailVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRawThmubnail * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRawThmubnail * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRawThmubnail * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRawThmubnail * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRawThmubnail * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRawThmubnail * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRawThmubnail * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } IRawThmubnailVtbl;

    interface IRawThmubnail
    {
        CONST_VTBL struct IRawThmubnailVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRawThmubnail_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IRawThmubnail_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IRawThmubnail_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IRawThmubnail_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IRawThmubnail_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IRawThmubnail_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IRawThmubnail_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IRawThmubnail_INTERFACE_DEFINED__ */



#ifndef __LSExifLib_LIBRARY_DEFINED__
#define __LSExifLib_LIBRARY_DEFINED__

/* library LSExifLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_LSExifLib;

EXTERN_C const CLSID CLSID_ExifParser;

#ifdef __cplusplus

class DECLSPEC_UUID("E5B0C991-E0F9-4CC2-812A-BDE422935E0F")
ExifParser;
#endif

EXTERN_C const CLSID CLSID_RawThmubnail;

#ifdef __cplusplus

class DECLSPEC_UUID("12524D4C-6E12-4615-987D-94BB7E1C1D8B")
RawThmubnail;
#endif
#endif /* __LSExifLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


