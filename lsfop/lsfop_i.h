

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 12:14:07 2038
 */
/* Compiler settings for lsfop.idl:
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

#ifndef __lsfop_i_h__
#define __lsfop_i_h__

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

#ifndef __ILSFileOperation_FWD_DEFINED__
#define __ILSFileOperation_FWD_DEFINED__
typedef interface ILSFileOperation ILSFileOperation;

#endif 	/* __ILSFileOperation_FWD_DEFINED__ */


#ifndef __LSFileOperation_FWD_DEFINED__
#define __LSFileOperation_FWD_DEFINED__

#ifdef __cplusplus
typedef class LSFileOperation LSFileOperation;
#else
typedef struct LSFileOperation LSFileOperation;
#endif /* __cplusplus */

#endif 	/* __LSFileOperation_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ILSFileOperation_INTERFACE_DEFINED__
#define __ILSFileOperation_INTERFACE_DEFINED__

/* interface ILSFileOperation */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_ILSFileOperation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8C51DB8F-392C-4FE8-A93B-B91AE9129D81")
    ILSFileOperation : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FileCopy( 
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR pszSrc,
            /* [in] */ LPCWSTR pszDst,
            /* [in] */ BOOL bCopy) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DropFileOperation( 
            /* [in] */ HWND hwnd,
            /* [in] */ HGLOBAL hdrop,
            /* [in] */ DWORD eff,
            /* [in] */ LPCWSTR pszDst,
            /* [in] */ BOOL bSaveFolderHierarchy) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE MakeTree( 
            /* [in] */ LPCWSTR pszDir) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE MakeTaskJob( 
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR pszSyncFile) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE MakeSymLink( 
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR lpTarget,
            /* [in] */ LPCWSTR lpSrc) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FileCopyEx( 
            /* [in] */ LPARAM param) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DropFileOperationEx( 
            /* [in] */ LPARAM param) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE MakeSymLinkUI( 
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR lpSymlinkBase,
            /* [in] */ LPCWSTR lpTargetFileName) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ILSFileOperationVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILSFileOperation * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILSFileOperation * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILSFileOperation * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILSFileOperation * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILSFileOperation * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILSFileOperation * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILSFileOperation * This,
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
        
        DECLSPEC_XFGVIRT(ILSFileOperation, FileCopy)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *FileCopy )( 
            ILSFileOperation * This,
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR pszSrc,
            /* [in] */ LPCWSTR pszDst,
            /* [in] */ BOOL bCopy);
        
        DECLSPEC_XFGVIRT(ILSFileOperation, DropFileOperation)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DropFileOperation )( 
            ILSFileOperation * This,
            /* [in] */ HWND hwnd,
            /* [in] */ HGLOBAL hdrop,
            /* [in] */ DWORD eff,
            /* [in] */ LPCWSTR pszDst,
            /* [in] */ BOOL bSaveFolderHierarchy);
        
        DECLSPEC_XFGVIRT(ILSFileOperation, MakeTree)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *MakeTree )( 
            ILSFileOperation * This,
            /* [in] */ LPCWSTR pszDir);
        
        DECLSPEC_XFGVIRT(ILSFileOperation, MakeTaskJob)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *MakeTaskJob )( 
            ILSFileOperation * This,
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR pszSyncFile);
        
        DECLSPEC_XFGVIRT(ILSFileOperation, MakeSymLink)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *MakeSymLink )( 
            ILSFileOperation * This,
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR lpTarget,
            /* [in] */ LPCWSTR lpSrc);
        
        DECLSPEC_XFGVIRT(ILSFileOperation, FileCopyEx)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *FileCopyEx )( 
            ILSFileOperation * This,
            /* [in] */ LPARAM param);
        
        DECLSPEC_XFGVIRT(ILSFileOperation, DropFileOperationEx)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DropFileOperationEx )( 
            ILSFileOperation * This,
            /* [in] */ LPARAM param);
        
        DECLSPEC_XFGVIRT(ILSFileOperation, MakeSymLinkUI)
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *MakeSymLinkUI )( 
            ILSFileOperation * This,
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR lpSymlinkBase,
            /* [in] */ LPCWSTR lpTargetFileName);
        
        END_INTERFACE
    } ILSFileOperationVtbl;

    interface ILSFileOperation
    {
        CONST_VTBL struct ILSFileOperationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILSFileOperation_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ILSFileOperation_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ILSFileOperation_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ILSFileOperation_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ILSFileOperation_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ILSFileOperation_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ILSFileOperation_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ILSFileOperation_FileCopy(This,hwnd,pszSrc,pszDst,bCopy)	\
    ( (This)->lpVtbl -> FileCopy(This,hwnd,pszSrc,pszDst,bCopy) ) 

#define ILSFileOperation_DropFileOperation(This,hwnd,hdrop,eff,pszDst,bSaveFolderHierarchy)	\
    ( (This)->lpVtbl -> DropFileOperation(This,hwnd,hdrop,eff,pszDst,bSaveFolderHierarchy) ) 

#define ILSFileOperation_MakeTree(This,pszDir)	\
    ( (This)->lpVtbl -> MakeTree(This,pszDir) ) 

#define ILSFileOperation_MakeTaskJob(This,hwnd,pszSyncFile)	\
    ( (This)->lpVtbl -> MakeTaskJob(This,hwnd,pszSyncFile) ) 

#define ILSFileOperation_MakeSymLink(This,hwnd,lpTarget,lpSrc)	\
    ( (This)->lpVtbl -> MakeSymLink(This,hwnd,lpTarget,lpSrc) ) 

#define ILSFileOperation_FileCopyEx(This,param)	\
    ( (This)->lpVtbl -> FileCopyEx(This,param) ) 

#define ILSFileOperation_DropFileOperationEx(This,param)	\
    ( (This)->lpVtbl -> DropFileOperationEx(This,param) ) 

#define ILSFileOperation_MakeSymLinkUI(This,hwnd,lpSymlinkBase,lpTargetFileName)	\
    ( (This)->lpVtbl -> MakeSymLinkUI(This,hwnd,lpSymlinkBase,lpTargetFileName) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ILSFileOperation_INTERFACE_DEFINED__ */



#ifndef __lsfopLib_LIBRARY_DEFINED__
#define __lsfopLib_LIBRARY_DEFINED__

/* library lsfopLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_lsfopLib;

EXTERN_C const CLSID CLSID_LSFileOperation;

#ifdef __cplusplus

class DECLSPEC_UUID("31EB20AB-9F41-42D9-8A00-CDCB43F9A56F")
LSFileOperation;
#endif
#endif /* __lsfopLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  HGLOBAL_UserSize(     unsigned long *, unsigned long            , HGLOBAL * ); 
unsigned char * __RPC_USER  HGLOBAL_UserMarshal(  unsigned long *, unsigned char *, HGLOBAL * ); 
unsigned char * __RPC_USER  HGLOBAL_UserUnmarshal(unsigned long *, unsigned char *, HGLOBAL * ); 
void                      __RPC_USER  HGLOBAL_UserFree(     unsigned long *, HGLOBAL * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

unsigned long             __RPC_USER  HGLOBAL_UserSize64(     unsigned long *, unsigned long            , HGLOBAL * ); 
unsigned char * __RPC_USER  HGLOBAL_UserMarshal64(  unsigned long *, unsigned char *, HGLOBAL * ); 
unsigned char * __RPC_USER  HGLOBAL_UserUnmarshal64(unsigned long *, unsigned char *, HGLOBAL * ); 
void                      __RPC_USER  HGLOBAL_UserFree64(     unsigned long *, HGLOBAL * ); 

unsigned long             __RPC_USER  HWND_UserSize64(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal64(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal64(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree64(     unsigned long *, HWND * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


