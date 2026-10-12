

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 12:14:07 2038
 */
/* Compiler settings for lsftc.idl:
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

#ifndef __lsftc_i_h__
#define __lsftc_i_h__

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

#ifndef __IFileXferComponent_FWD_DEFINED__
#define __IFileXferComponent_FWD_DEFINED__
typedef interface IFileXferComponent IFileXferComponent;

#endif 	/* __IFileXferComponent_FWD_DEFINED__ */


#ifndef __FileXferComponent_FWD_DEFINED__
#define __FileXferComponent_FWD_DEFINED__

#ifdef __cplusplus
typedef class FileXferComponent FileXferComponent;
#else
typedef struct FileXferComponent FileXferComponent;
#endif /* __cplusplus */

#endif 	/* __FileXferComponent_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IFileXferComponent_INTERFACE_DEFINED__
#define __IFileXferComponent_INTERFACE_DEFINED__

/* interface IFileXferComponent */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IFileXferComponent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C39D1965-E74C-45B0-9FBA-F886AD854FDB")
    IFileXferComponent : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetInstance( 
            /* [retval][out] */ LONG *val) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ResetInstance( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE InitInstance( void) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IFileXferComponentVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFileXferComponent * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFileXferComponent * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFileXferComponent * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFileXferComponent * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFileXferComponent * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFileXferComponent * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFileXferComponent * This,
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
        
        DECLSPEC_XFGVIRT(IFileXferComponent, GetInstance)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetInstance )( 
            IFileXferComponent * This,
            /* [retval][out] */ LONG *val);
        
        DECLSPEC_XFGVIRT(IFileXferComponent, ResetInstance)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *ResetInstance )( 
            IFileXferComponent * This);
        
        DECLSPEC_XFGVIRT(IFileXferComponent, InitInstance)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *InitInstance )( 
            IFileXferComponent * This);
        
        END_INTERFACE
    } IFileXferComponentVtbl;

    interface IFileXferComponent
    {
        CONST_VTBL struct IFileXferComponentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFileXferComponent_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IFileXferComponent_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IFileXferComponent_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IFileXferComponent_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IFileXferComponent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IFileXferComponent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IFileXferComponent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IFileXferComponent_GetInstance(This,val)	\
    ( (This)->lpVtbl -> GetInstance(This,val) ) 

#define IFileXferComponent_ResetInstance(This)	\
    ( (This)->lpVtbl -> ResetInstance(This) ) 

#define IFileXferComponent_InitInstance(This)	\
    ( (This)->lpVtbl -> InitInstance(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IFileXferComponent_INTERFACE_DEFINED__ */



#ifndef __lsftcLib_LIBRARY_DEFINED__
#define __lsftcLib_LIBRARY_DEFINED__

/* library lsftcLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_lsftcLib;

EXTERN_C const CLSID CLSID_FileXferComponent;

#ifdef __cplusplus

class DECLSPEC_UUID("49477A47-1CBD-40FC-B62A-AE8220F10B09")
FileXferComponent;
#endif
#endif /* __lsftcLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


