

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 12:14:07 2038
 */
/* Compiler settings for lsarcsvr.idl:
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

#ifndef __lsarcsvr_i_h__
#define __lsarcsvr_i_h__

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

#ifndef __ICommArchiver_FWD_DEFINED__
#define __ICommArchiver_FWD_DEFINED__
typedef interface ICommArchiver ICommArchiver;

#endif 	/* __ICommArchiver_FWD_DEFINED__ */


#ifndef __IArcManager_FWD_DEFINED__
#define __IArcManager_FWD_DEFINED__
typedef interface IArcManager IArcManager;

#endif 	/* __IArcManager_FWD_DEFINED__ */


#ifndef __CommArchiver_FWD_DEFINED__
#define __CommArchiver_FWD_DEFINED__

#ifdef __cplusplus
typedef class CommArchiver CommArchiver;
#else
typedef struct CommArchiver CommArchiver;
#endif /* __cplusplus */

#endif 	/* __CommArchiver_FWD_DEFINED__ */


#ifndef __ArcManager_FWD_DEFINED__
#define __ArcManager_FWD_DEFINED__

#ifdef __cplusplus
typedef class ArcManager ArcManager;
#else
typedef struct ArcManager ArcManager;
#endif /* __cplusplus */

#endif 	/* __ArcManager_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ICommArchiver_INTERFACE_DEFINED__
#define __ICommArchiver_INTERFACE_DEFINED__

/* interface ICommArchiver */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_ICommArchiver;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BECA4F33-DA70-489C-8AB7-C769044CAED5")
    ICommArchiver : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Initialize( 
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR pszPrefix,
            /* [in] */ LPCWSTR pszDll) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Locked( 
            /* [in] */ DWORD dwMarker) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Command( 
            /* [in] */ LPCWSTR pCmd,
            /* [out] */ CHAR *pResultBuf,
            /* [in] */ ULONG sizeResult) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CommandA( 
            /* [in] */ LPCSTR pCmd,
            /* [out] */ CHAR *pResultBuf,
            /* [in] */ ULONG sizeResult) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CommandW( 
            /* [in] */ LPCWSTR pCmd,
            /* [out] */ WCHAR *pResultBuf,
            /* [in] */ ULONG sizeResult) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetVersion( 
            /* [retval][out] */ WORD *pwVersion) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE OpenArchive( 
            /* [in] */ LPCWSTR pszFile,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CloseArchive( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE FindFirst( 
            /* [in] */ LPCWSTR pszFilter,
            /* [out] */ BYTE *piia) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE FindFirstW( 
            /* [in] */ LPCWSTR pszFilter,
            /* [out] */ BYTE *piiw) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE FindNext( 
            /* [out] */ BYTE *piia) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE FindNextW( 
            /* [out] */ BYTE *piiw) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetAttribute( 
            /* [retval][out] */ DWORD *pdwAttr) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetArcFileName( 
            /* [out] */ WCHAR *pszBuffer,
            /* [in] */ LONG cchMax) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetArcFileSize( 
            /* [retval][out] */ DWORD *pdwSize) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetArcOriginalSize( 
            /* [retval][out] */ DWORD *pdwSize) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetArcCompressedSize( 
            /* [retval][out] */ DWORD *pdwSize) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetArcRatio( 
            /* [retval][out] */ WORD *pwRatio) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetArcDateTime( 
            /* [out] */ FILETIME *ft) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetArcOSType( 
            /* [retval][out] */ LONG *plType) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetArcCreateTimeEx( 
            /* [out] */ FILETIME *ft) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetArcAccessTimeEx( 
            /* [out] */ FILETIME *ft) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetArcWriteTimeEx( 
            /* [out] */ FILETIME *ft) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetFileName( 
            /* [out] */ WCHAR *pszName,
            /* [in] */ LONG cchMax) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetMethod( 
            /* [out] */ WCHAR *pszName,
            /* [in] */ LONG cchMax) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetOriginalSize( 
            /* [retval][out] */ DWORD *pdwSize) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetCompressedSize( 
            /* [retval][out] */ DWORD *pdwSize) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetRatio( 
            /* [retval][out] */ WORD *pwRatio) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetDateTime( 
            /* [out] */ FILETIME *ft) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetWriteTime( 
            /* [retval][out] */ LONG *tm) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetCreateTime( 
            /* [retval][out] */ LONG *tm) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetAccessTime( 
            /* [retval][out] */ LONG *tm) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetCRC( 
            /* [retval][out] */ DWORD *pdwCRC) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetOSType( 
            /* [retval][out] */ ULONG *plType) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetWriteTimeEx( 
            /* [out] */ FILETIME *ft) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetCreateTimeEx( 
            /* [out] */ FILETIME *ft) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetAccessTimeEx( 
            /* [out] */ FILETIME *ft) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SetBkgndMode( 
            /* [in] */ BOOL bBkgnd) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SetCursorMode( 
            /* [in] */ BOOL bOn) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE IsSFXFile( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CheckArchive( 
            /* [in] */ LPCWSTR pszFile,
            /* [in] */ ULONG mode) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SetOwnerWindow( 
            /* [in] */ HWND hwnd) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ClearOwnerWindow( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SetOwnerWindowEx( 
            /* [in] */ HWND hwnd,
            /* [in] */ LONG_PTR pfnArcProc) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE KillOwnerWindowEx( 
            /* [in] */ HWND hwnd) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetLastError( 
            /* [retval][out] */ LONG *plCode) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ICommArchiverVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICommArchiver * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICommArchiver * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICommArchiver * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICommArchiver * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICommArchiver * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICommArchiver * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICommArchiver * This,
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
        
        DECLSPEC_XFGVIRT(ICommArchiver, Initialize)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ICommArchiver * This,
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR pszPrefix,
            /* [in] */ LPCWSTR pszDll);
        
        DECLSPEC_XFGVIRT(ICommArchiver, Locked)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Locked )( 
            ICommArchiver * This,
            /* [in] */ DWORD dwMarker);
        
        DECLSPEC_XFGVIRT(ICommArchiver, Command)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Command )( 
            ICommArchiver * This,
            /* [in] */ LPCWSTR pCmd,
            /* [out] */ CHAR *pResultBuf,
            /* [in] */ ULONG sizeResult);
        
        DECLSPEC_XFGVIRT(ICommArchiver, CommandA)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CommandA )( 
            ICommArchiver * This,
            /* [in] */ LPCSTR pCmd,
            /* [out] */ CHAR *pResultBuf,
            /* [in] */ ULONG sizeResult);
        
        DECLSPEC_XFGVIRT(ICommArchiver, CommandW)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CommandW )( 
            ICommArchiver * This,
            /* [in] */ LPCWSTR pCmd,
            /* [out] */ WCHAR *pResultBuf,
            /* [in] */ ULONG sizeResult);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetVersion)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetVersion )( 
            ICommArchiver * This,
            /* [retval][out] */ WORD *pwVersion);
        
        DECLSPEC_XFGVIRT(ICommArchiver, OpenArchive)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *OpenArchive )( 
            ICommArchiver * This,
            /* [in] */ LPCWSTR pszFile,
            /* [in] */ DWORD dwFlags);
        
        DECLSPEC_XFGVIRT(ICommArchiver, CloseArchive)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CloseArchive )( 
            ICommArchiver * This);
        
        DECLSPEC_XFGVIRT(ICommArchiver, FindFirst)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *FindFirst )( 
            ICommArchiver * This,
            /* [in] */ LPCWSTR pszFilter,
            /* [out] */ BYTE *piia);
        
        DECLSPEC_XFGVIRT(ICommArchiver, FindFirstW)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *FindFirstW )( 
            ICommArchiver * This,
            /* [in] */ LPCWSTR pszFilter,
            /* [out] */ BYTE *piiw);
        
        DECLSPEC_XFGVIRT(ICommArchiver, FindNext)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *FindNext )( 
            ICommArchiver * This,
            /* [out] */ BYTE *piia);
        
        DECLSPEC_XFGVIRT(ICommArchiver, FindNextW)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *FindNextW )( 
            ICommArchiver * This,
            /* [out] */ BYTE *piiw);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetAttribute)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetAttribute )( 
            ICommArchiver * This,
            /* [retval][out] */ DWORD *pdwAttr);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetArcFileName)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetArcFileName )( 
            ICommArchiver * This,
            /* [out] */ WCHAR *pszBuffer,
            /* [in] */ LONG cchMax);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetArcFileSize)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetArcFileSize )( 
            ICommArchiver * This,
            /* [retval][out] */ DWORD *pdwSize);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetArcOriginalSize)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetArcOriginalSize )( 
            ICommArchiver * This,
            /* [retval][out] */ DWORD *pdwSize);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetArcCompressedSize)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetArcCompressedSize )( 
            ICommArchiver * This,
            /* [retval][out] */ DWORD *pdwSize);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetArcRatio)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetArcRatio )( 
            ICommArchiver * This,
            /* [retval][out] */ WORD *pwRatio);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetArcDateTime)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetArcDateTime )( 
            ICommArchiver * This,
            /* [out] */ FILETIME *ft);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetArcOSType)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetArcOSType )( 
            ICommArchiver * This,
            /* [retval][out] */ LONG *plType);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetArcCreateTimeEx)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetArcCreateTimeEx )( 
            ICommArchiver * This,
            /* [out] */ FILETIME *ft);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetArcAccessTimeEx)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetArcAccessTimeEx )( 
            ICommArchiver * This,
            /* [out] */ FILETIME *ft);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetArcWriteTimeEx)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetArcWriteTimeEx )( 
            ICommArchiver * This,
            /* [out] */ FILETIME *ft);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetFileName)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetFileName )( 
            ICommArchiver * This,
            /* [out] */ WCHAR *pszName,
            /* [in] */ LONG cchMax);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetMethod)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetMethod )( 
            ICommArchiver * This,
            /* [out] */ WCHAR *pszName,
            /* [in] */ LONG cchMax);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetOriginalSize)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetOriginalSize )( 
            ICommArchiver * This,
            /* [retval][out] */ DWORD *pdwSize);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetCompressedSize)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetCompressedSize )( 
            ICommArchiver * This,
            /* [retval][out] */ DWORD *pdwSize);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetRatio)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetRatio )( 
            ICommArchiver * This,
            /* [retval][out] */ WORD *pwRatio);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetDateTime)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetDateTime )( 
            ICommArchiver * This,
            /* [out] */ FILETIME *ft);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetWriteTime)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetWriteTime )( 
            ICommArchiver * This,
            /* [retval][out] */ LONG *tm);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetCreateTime)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetCreateTime )( 
            ICommArchiver * This,
            /* [retval][out] */ LONG *tm);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetAccessTime)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetAccessTime )( 
            ICommArchiver * This,
            /* [retval][out] */ LONG *tm);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetCRC)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetCRC )( 
            ICommArchiver * This,
            /* [retval][out] */ DWORD *pdwCRC);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetOSType)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetOSType )( 
            ICommArchiver * This,
            /* [retval][out] */ ULONG *plType);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetWriteTimeEx)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetWriteTimeEx )( 
            ICommArchiver * This,
            /* [out] */ FILETIME *ft);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetCreateTimeEx)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetCreateTimeEx )( 
            ICommArchiver * This,
            /* [out] */ FILETIME *ft);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetAccessTimeEx)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetAccessTimeEx )( 
            ICommArchiver * This,
            /* [out] */ FILETIME *ft);
        
        DECLSPEC_XFGVIRT(ICommArchiver, SetBkgndMode)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *SetBkgndMode )( 
            ICommArchiver * This,
            /* [in] */ BOOL bBkgnd);
        
        DECLSPEC_XFGVIRT(ICommArchiver, SetCursorMode)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *SetCursorMode )( 
            ICommArchiver * This,
            /* [in] */ BOOL bOn);
        
        DECLSPEC_XFGVIRT(ICommArchiver, IsSFXFile)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *IsSFXFile )( 
            ICommArchiver * This);
        
        DECLSPEC_XFGVIRT(ICommArchiver, CheckArchive)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CheckArchive )( 
            ICommArchiver * This,
            /* [in] */ LPCWSTR pszFile,
            /* [in] */ ULONG mode);
        
        DECLSPEC_XFGVIRT(ICommArchiver, SetOwnerWindow)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *SetOwnerWindow )( 
            ICommArchiver * This,
            /* [in] */ HWND hwnd);
        
        DECLSPEC_XFGVIRT(ICommArchiver, ClearOwnerWindow)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *ClearOwnerWindow )( 
            ICommArchiver * This);
        
        DECLSPEC_XFGVIRT(ICommArchiver, SetOwnerWindowEx)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *SetOwnerWindowEx )( 
            ICommArchiver * This,
            /* [in] */ HWND hwnd,
            /* [in] */ LONG_PTR pfnArcProc);
        
        DECLSPEC_XFGVIRT(ICommArchiver, KillOwnerWindowEx)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *KillOwnerWindowEx )( 
            ICommArchiver * This,
            /* [in] */ HWND hwnd);
        
        DECLSPEC_XFGVIRT(ICommArchiver, GetLastError)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetLastError )( 
            ICommArchiver * This,
            /* [retval][out] */ LONG *plCode);
        
        END_INTERFACE
    } ICommArchiverVtbl;

    interface ICommArchiver
    {
        CONST_VTBL struct ICommArchiverVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommArchiver_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICommArchiver_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICommArchiver_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICommArchiver_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ICommArchiver_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ICommArchiver_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ICommArchiver_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ICommArchiver_Initialize(This,hwnd,pszPrefix,pszDll)	\
    ( (This)->lpVtbl -> Initialize(This,hwnd,pszPrefix,pszDll) ) 

#define ICommArchiver_Locked(This,dwMarker)	\
    ( (This)->lpVtbl -> Locked(This,dwMarker) ) 

#define ICommArchiver_Command(This,pCmd,pResultBuf,sizeResult)	\
    ( (This)->lpVtbl -> Command(This,pCmd,pResultBuf,sizeResult) ) 

#define ICommArchiver_CommandA(This,pCmd,pResultBuf,sizeResult)	\
    ( (This)->lpVtbl -> CommandA(This,pCmd,pResultBuf,sizeResult) ) 

#define ICommArchiver_CommandW(This,pCmd,pResultBuf,sizeResult)	\
    ( (This)->lpVtbl -> CommandW(This,pCmd,pResultBuf,sizeResult) ) 

#define ICommArchiver_GetVersion(This,pwVersion)	\
    ( (This)->lpVtbl -> GetVersion(This,pwVersion) ) 

#define ICommArchiver_OpenArchive(This,pszFile,dwFlags)	\
    ( (This)->lpVtbl -> OpenArchive(This,pszFile,dwFlags) ) 

#define ICommArchiver_CloseArchive(This)	\
    ( (This)->lpVtbl -> CloseArchive(This) ) 

#define ICommArchiver_FindFirst(This,pszFilter,piia)	\
    ( (This)->lpVtbl -> FindFirst(This,pszFilter,piia) ) 

#define ICommArchiver_FindFirstW(This,pszFilter,piiw)	\
    ( (This)->lpVtbl -> FindFirstW(This,pszFilter,piiw) ) 

#define ICommArchiver_FindNext(This,piia)	\
    ( (This)->lpVtbl -> FindNext(This,piia) ) 

#define ICommArchiver_FindNextW(This,piiw)	\
    ( (This)->lpVtbl -> FindNextW(This,piiw) ) 

#define ICommArchiver_GetAttribute(This,pdwAttr)	\
    ( (This)->lpVtbl -> GetAttribute(This,pdwAttr) ) 

#define ICommArchiver_GetArcFileName(This,pszBuffer,cchMax)	\
    ( (This)->lpVtbl -> GetArcFileName(This,pszBuffer,cchMax) ) 

#define ICommArchiver_GetArcFileSize(This,pdwSize)	\
    ( (This)->lpVtbl -> GetArcFileSize(This,pdwSize) ) 

#define ICommArchiver_GetArcOriginalSize(This,pdwSize)	\
    ( (This)->lpVtbl -> GetArcOriginalSize(This,pdwSize) ) 

#define ICommArchiver_GetArcCompressedSize(This,pdwSize)	\
    ( (This)->lpVtbl -> GetArcCompressedSize(This,pdwSize) ) 

#define ICommArchiver_GetArcRatio(This,pwRatio)	\
    ( (This)->lpVtbl -> GetArcRatio(This,pwRatio) ) 

#define ICommArchiver_GetArcDateTime(This,ft)	\
    ( (This)->lpVtbl -> GetArcDateTime(This,ft) ) 

#define ICommArchiver_GetArcOSType(This,plType)	\
    ( (This)->lpVtbl -> GetArcOSType(This,plType) ) 

#define ICommArchiver_GetArcCreateTimeEx(This,ft)	\
    ( (This)->lpVtbl -> GetArcCreateTimeEx(This,ft) ) 

#define ICommArchiver_GetArcAccessTimeEx(This,ft)	\
    ( (This)->lpVtbl -> GetArcAccessTimeEx(This,ft) ) 

#define ICommArchiver_GetArcWriteTimeEx(This,ft)	\
    ( (This)->lpVtbl -> GetArcWriteTimeEx(This,ft) ) 

#define ICommArchiver_GetFileName(This,pszName,cchMax)	\
    ( (This)->lpVtbl -> GetFileName(This,pszName,cchMax) ) 

#define ICommArchiver_GetMethod(This,pszName,cchMax)	\
    ( (This)->lpVtbl -> GetMethod(This,pszName,cchMax) ) 

#define ICommArchiver_GetOriginalSize(This,pdwSize)	\
    ( (This)->lpVtbl -> GetOriginalSize(This,pdwSize) ) 

#define ICommArchiver_GetCompressedSize(This,pdwSize)	\
    ( (This)->lpVtbl -> GetCompressedSize(This,pdwSize) ) 

#define ICommArchiver_GetRatio(This,pwRatio)	\
    ( (This)->lpVtbl -> GetRatio(This,pwRatio) ) 

#define ICommArchiver_GetDateTime(This,ft)	\
    ( (This)->lpVtbl -> GetDateTime(This,ft) ) 

#define ICommArchiver_GetWriteTime(This,tm)	\
    ( (This)->lpVtbl -> GetWriteTime(This,tm) ) 

#define ICommArchiver_GetCreateTime(This,tm)	\
    ( (This)->lpVtbl -> GetCreateTime(This,tm) ) 

#define ICommArchiver_GetAccessTime(This,tm)	\
    ( (This)->lpVtbl -> GetAccessTime(This,tm) ) 

#define ICommArchiver_GetCRC(This,pdwCRC)	\
    ( (This)->lpVtbl -> GetCRC(This,pdwCRC) ) 

#define ICommArchiver_GetOSType(This,plType)	\
    ( (This)->lpVtbl -> GetOSType(This,plType) ) 

#define ICommArchiver_GetWriteTimeEx(This,ft)	\
    ( (This)->lpVtbl -> GetWriteTimeEx(This,ft) ) 

#define ICommArchiver_GetCreateTimeEx(This,ft)	\
    ( (This)->lpVtbl -> GetCreateTimeEx(This,ft) ) 

#define ICommArchiver_GetAccessTimeEx(This,ft)	\
    ( (This)->lpVtbl -> GetAccessTimeEx(This,ft) ) 

#define ICommArchiver_SetBkgndMode(This,bBkgnd)	\
    ( (This)->lpVtbl -> SetBkgndMode(This,bBkgnd) ) 

#define ICommArchiver_SetCursorMode(This,bOn)	\
    ( (This)->lpVtbl -> SetCursorMode(This,bOn) ) 

#define ICommArchiver_IsSFXFile(This)	\
    ( (This)->lpVtbl -> IsSFXFile(This) ) 

#define ICommArchiver_CheckArchive(This,pszFile,mode)	\
    ( (This)->lpVtbl -> CheckArchive(This,pszFile,mode) ) 

#define ICommArchiver_SetOwnerWindow(This,hwnd)	\
    ( (This)->lpVtbl -> SetOwnerWindow(This,hwnd) ) 

#define ICommArchiver_ClearOwnerWindow(This)	\
    ( (This)->lpVtbl -> ClearOwnerWindow(This) ) 

#define ICommArchiver_SetOwnerWindowEx(This,hwnd,pfnArcProc)	\
    ( (This)->lpVtbl -> SetOwnerWindowEx(This,hwnd,pfnArcProc) ) 

#define ICommArchiver_KillOwnerWindowEx(This,hwnd)	\
    ( (This)->lpVtbl -> KillOwnerWindowEx(This,hwnd) ) 

#define ICommArchiver_GetLastError(This,plCode)	\
    ( (This)->lpVtbl -> GetLastError(This,plCode) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICommArchiver_INTERFACE_DEFINED__ */


#ifndef __IArcManager_INTERFACE_DEFINED__
#define __IArcManager_INTERFACE_DEFINED__

/* interface IArcManager */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IArcManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C5B0F895-28CE-4063-8E32-8C122B52DBCD")
    IArcManager : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Archive( 
            /* [in] */ HWND hwnd,
            /* [in] */ ULONG type,
            /* [in] */ LPCWSTR pszCmd,
            /* [out] */ CHAR *pszResult,
            /* [in] */ ULONG sizeResult,
            /* [retval][out] */ LONG *plArcError) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ArchiveW( 
            /* [in] */ HWND hwnd,
            /* [in] */ ULONG type,
            /* [in] */ LPCWSTR pszCmd,
            /* [out] */ WCHAR *pszResult,
            /* [in] */ ULONG sizeResult,
            /* [retval][out] */ LONG *plArcError) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE AddFiles( 
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR pszArc,
            /* [in] */ LPCWSTR pszBase,
            /* [in] */ LONG_PTR hdrop_ptr) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CheckFiles( 
            /* [in] */ HWND hwnd,
            /* [in] */ LONG_PTR hdrop_ptr) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CompressFiles( 
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR pszBase,
            /* [in] */ LONG_PTR hdrop) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CompressFilesExec( 
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR pszBase,
            /* [in] */ LONG_PTR hdrop) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ExtractFiles( 
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR pszPath,
            /* [in] */ LONG_PTR hdrop_ptr) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ExtractFilesExec( 
            HWND hwnd,
            /* [in] */ LPCWSTR pszPath,
            /* [in] */ LONG_PTR hdrop_ptr) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ExtractFromArchive( 
            /* [in] */ HWND hwnd,
            /* [in] */ ULONG type,
            /* [in] */ LPCWSTR pszFile,
            /* [in] */ LPCWSTR pszList) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE BeginFind( 
            /* [in] */ HWND hwnd,
            /* [in] */ ULONG type,
            /* [in] */ LPCWSTR pszFile,
            /* [in] */ LPCWSTR pszFilter,
            /* [out] */ BYTE *piiw,
            /* [out] */ IUnknown **ppInterface) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE NextFind( 
            /* [in] */ IUnknown *pInterface,
            /* [out] */ BYTE *piiw) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE EndFind( 
            /* [in] */ IUnknown *pInterface) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetArchiveType( 
            /* [in] */ LPCWSTR pszFile,
            /* [out] */ LONG *type,
            /* [out] */ BOOL *pbUnicodeSupported,
            /* [out] */ BOOL *pbUtf8Supported) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetDefaultFolder( 
            /* [out] */ WCHAR *pszFolder) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SetDefaultFolder( 
            /* [in] */ LPCWSTR pszFolder) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE MakeSFX( 
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR pszFile,
            /* [in] */ LPCWSTR pszTargetPath) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE IsArchive( 
            /* [in] */ HWND hwnd,
            /* [in] */ ULONG type,
            /* [in] */ LPCWSTR pszArc) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE IsLibraryAvailable( 
            /* [in] */ ULONG type) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ListArchive( 
            /* [in] */ HWND hwnd,
            /* [in] */ ULONG type,
            /* [in] */ LPCWSTR pszFile,
            /* [out] */ CHAR *pszResult,
            /* [in] */ ULONG size) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IArcManagerVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IArcManager * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IArcManager * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IArcManager * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IArcManager * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IArcManager * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IArcManager * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IArcManager * This,
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
        
        DECLSPEC_XFGVIRT(IArcManager, Archive)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Archive )( 
            IArcManager * This,
            /* [in] */ HWND hwnd,
            /* [in] */ ULONG type,
            /* [in] */ LPCWSTR pszCmd,
            /* [out] */ CHAR *pszResult,
            /* [in] */ ULONG sizeResult,
            /* [retval][out] */ LONG *plArcError);
        
        DECLSPEC_XFGVIRT(IArcManager, ArchiveW)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *ArchiveW )( 
            IArcManager * This,
            /* [in] */ HWND hwnd,
            /* [in] */ ULONG type,
            /* [in] */ LPCWSTR pszCmd,
            /* [out] */ WCHAR *pszResult,
            /* [in] */ ULONG sizeResult,
            /* [retval][out] */ LONG *plArcError);
        
        DECLSPEC_XFGVIRT(IArcManager, AddFiles)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *AddFiles )( 
            IArcManager * This,
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR pszArc,
            /* [in] */ LPCWSTR pszBase,
            /* [in] */ LONG_PTR hdrop_ptr);
        
        DECLSPEC_XFGVIRT(IArcManager, CheckFiles)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CheckFiles )( 
            IArcManager * This,
            /* [in] */ HWND hwnd,
            /* [in] */ LONG_PTR hdrop_ptr);
        
        DECLSPEC_XFGVIRT(IArcManager, CompressFiles)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CompressFiles )( 
            IArcManager * This,
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR pszBase,
            /* [in] */ LONG_PTR hdrop);
        
        DECLSPEC_XFGVIRT(IArcManager, CompressFilesExec)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CompressFilesExec )( 
            IArcManager * This,
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR pszBase,
            /* [in] */ LONG_PTR hdrop);
        
        DECLSPEC_XFGVIRT(IArcManager, ExtractFiles)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *ExtractFiles )( 
            IArcManager * This,
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR pszPath,
            /* [in] */ LONG_PTR hdrop_ptr);
        
        DECLSPEC_XFGVIRT(IArcManager, ExtractFilesExec)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *ExtractFilesExec )( 
            IArcManager * This,
            HWND hwnd,
            /* [in] */ LPCWSTR pszPath,
            /* [in] */ LONG_PTR hdrop_ptr);
        
        DECLSPEC_XFGVIRT(IArcManager, ExtractFromArchive)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *ExtractFromArchive )( 
            IArcManager * This,
            /* [in] */ HWND hwnd,
            /* [in] */ ULONG type,
            /* [in] */ LPCWSTR pszFile,
            /* [in] */ LPCWSTR pszList);
        
        DECLSPEC_XFGVIRT(IArcManager, BeginFind)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *BeginFind )( 
            IArcManager * This,
            /* [in] */ HWND hwnd,
            /* [in] */ ULONG type,
            /* [in] */ LPCWSTR pszFile,
            /* [in] */ LPCWSTR pszFilter,
            /* [out] */ BYTE *piiw,
            /* [out] */ IUnknown **ppInterface);
        
        DECLSPEC_XFGVIRT(IArcManager, NextFind)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *NextFind )( 
            IArcManager * This,
            /* [in] */ IUnknown *pInterface,
            /* [out] */ BYTE *piiw);
        
        DECLSPEC_XFGVIRT(IArcManager, EndFind)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *EndFind )( 
            IArcManager * This,
            /* [in] */ IUnknown *pInterface);
        
        DECLSPEC_XFGVIRT(IArcManager, GetArchiveType)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetArchiveType )( 
            IArcManager * This,
            /* [in] */ LPCWSTR pszFile,
            /* [out] */ LONG *type,
            /* [out] */ BOOL *pbUnicodeSupported,
            /* [out] */ BOOL *pbUtf8Supported);
        
        DECLSPEC_XFGVIRT(IArcManager, GetDefaultFolder)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetDefaultFolder )( 
            IArcManager * This,
            /* [out] */ WCHAR *pszFolder);
        
        DECLSPEC_XFGVIRT(IArcManager, SetDefaultFolder)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *SetDefaultFolder )( 
            IArcManager * This,
            /* [in] */ LPCWSTR pszFolder);
        
        DECLSPEC_XFGVIRT(IArcManager, MakeSFX)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *MakeSFX )( 
            IArcManager * This,
            /* [in] */ HWND hwnd,
            /* [in] */ LPCWSTR pszFile,
            /* [in] */ LPCWSTR pszTargetPath);
        
        DECLSPEC_XFGVIRT(IArcManager, IsArchive)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *IsArchive )( 
            IArcManager * This,
            /* [in] */ HWND hwnd,
            /* [in] */ ULONG type,
            /* [in] */ LPCWSTR pszArc);
        
        DECLSPEC_XFGVIRT(IArcManager, IsLibraryAvailable)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *IsLibraryAvailable )( 
            IArcManager * This,
            /* [in] */ ULONG type);
        
        DECLSPEC_XFGVIRT(IArcManager, ListArchive)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *ListArchive )( 
            IArcManager * This,
            /* [in] */ HWND hwnd,
            /* [in] */ ULONG type,
            /* [in] */ LPCWSTR pszFile,
            /* [out] */ CHAR *pszResult,
            /* [in] */ ULONG size);
        
        END_INTERFACE
    } IArcManagerVtbl;

    interface IArcManager
    {
        CONST_VTBL struct IArcManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IArcManager_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IArcManager_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IArcManager_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IArcManager_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IArcManager_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IArcManager_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IArcManager_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IArcManager_Archive(This,hwnd,type,pszCmd,pszResult,sizeResult,plArcError)	\
    ( (This)->lpVtbl -> Archive(This,hwnd,type,pszCmd,pszResult,sizeResult,plArcError) ) 

#define IArcManager_ArchiveW(This,hwnd,type,pszCmd,pszResult,sizeResult,plArcError)	\
    ( (This)->lpVtbl -> ArchiveW(This,hwnd,type,pszCmd,pszResult,sizeResult,plArcError) ) 

#define IArcManager_AddFiles(This,hwnd,pszArc,pszBase,hdrop_ptr)	\
    ( (This)->lpVtbl -> AddFiles(This,hwnd,pszArc,pszBase,hdrop_ptr) ) 

#define IArcManager_CheckFiles(This,hwnd,hdrop_ptr)	\
    ( (This)->lpVtbl -> CheckFiles(This,hwnd,hdrop_ptr) ) 

#define IArcManager_CompressFiles(This,hwnd,pszBase,hdrop)	\
    ( (This)->lpVtbl -> CompressFiles(This,hwnd,pszBase,hdrop) ) 

#define IArcManager_CompressFilesExec(This,hwnd,pszBase,hdrop)	\
    ( (This)->lpVtbl -> CompressFilesExec(This,hwnd,pszBase,hdrop) ) 

#define IArcManager_ExtractFiles(This,hwnd,pszPath,hdrop_ptr)	\
    ( (This)->lpVtbl -> ExtractFiles(This,hwnd,pszPath,hdrop_ptr) ) 

#define IArcManager_ExtractFilesExec(This,hwnd,pszPath,hdrop_ptr)	\
    ( (This)->lpVtbl -> ExtractFilesExec(This,hwnd,pszPath,hdrop_ptr) ) 

#define IArcManager_ExtractFromArchive(This,hwnd,type,pszFile,pszList)	\
    ( (This)->lpVtbl -> ExtractFromArchive(This,hwnd,type,pszFile,pszList) ) 

#define IArcManager_BeginFind(This,hwnd,type,pszFile,pszFilter,piiw,ppInterface)	\
    ( (This)->lpVtbl -> BeginFind(This,hwnd,type,pszFile,pszFilter,piiw,ppInterface) ) 

#define IArcManager_NextFind(This,pInterface,piiw)	\
    ( (This)->lpVtbl -> NextFind(This,pInterface,piiw) ) 

#define IArcManager_EndFind(This,pInterface)	\
    ( (This)->lpVtbl -> EndFind(This,pInterface) ) 

#define IArcManager_GetArchiveType(This,pszFile,type,pbUnicodeSupported,pbUtf8Supported)	\
    ( (This)->lpVtbl -> GetArchiveType(This,pszFile,type,pbUnicodeSupported,pbUtf8Supported) ) 

#define IArcManager_GetDefaultFolder(This,pszFolder)	\
    ( (This)->lpVtbl -> GetDefaultFolder(This,pszFolder) ) 

#define IArcManager_SetDefaultFolder(This,pszFolder)	\
    ( (This)->lpVtbl -> SetDefaultFolder(This,pszFolder) ) 

#define IArcManager_MakeSFX(This,hwnd,pszFile,pszTargetPath)	\
    ( (This)->lpVtbl -> MakeSFX(This,hwnd,pszFile,pszTargetPath) ) 

#define IArcManager_IsArchive(This,hwnd,type,pszArc)	\
    ( (This)->lpVtbl -> IsArchive(This,hwnd,type,pszArc) ) 

#define IArcManager_IsLibraryAvailable(This,type)	\
    ( (This)->lpVtbl -> IsLibraryAvailable(This,type) ) 

#define IArcManager_ListArchive(This,hwnd,type,pszFile,pszResult,size)	\
    ( (This)->lpVtbl -> ListArchive(This,hwnd,type,pszFile,pszResult,size) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IArcManager_INTERFACE_DEFINED__ */



#ifndef __lsarcsvrLib_LIBRARY_DEFINED__
#define __lsarcsvrLib_LIBRARY_DEFINED__

/* library lsarcsvrLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_lsarcsvrLib;

EXTERN_C const CLSID CLSID_CommArchiver;

#ifdef __cplusplus

class DECLSPEC_UUID("26D6C970-A507-4943-B2D5-D0D1AC4EA7DF")
CommArchiver;
#endif

EXTERN_C const CLSID CLSID_ArcManager;

#ifdef __cplusplus

class DECLSPEC_UUID("E51300C0-DF07-4ED4-B629-2F3458A38496")
ArcManager;
#endif
#endif /* __lsarcsvrLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

unsigned long             __RPC_USER  HWND_UserSize64(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal64(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal64(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree64(     unsigned long *, HWND * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


