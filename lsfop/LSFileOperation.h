// LSFileOperation.h : CLSFileOperation の宣言

#pragma once
#include "resource.h"       // メイン シンボル

#include "lsfop_i.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "DCOM の完全サポートを含んでいない Windows Mobile プラットフォームのような Windows CE プラットフォームでは、単一スレッド COM オブジェクトは正しくサポートされていません。ATL が単一スレッド COM オブジェクトの作成をサポートすること、およびその単一スレッド COM オブジェクトの実装の使用を許可することを強制するには、_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA を定義してください。ご使用の rgs ファイルのスレッド モデルは 'Free' に設定されており、DCOM Windows CE 以外のプラットフォームでサポートされる唯一のスレッド モデルと設定されていました。"
#endif



// CLSFileOperation

class ATL_NO_VTABLE CLSFileOperation :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CLSFileOperation, &CLSID_LSFileOperation>,
	public IDispatchImpl<ILSFileOperation, &IID_ILSFileOperation, &LIBID_lsfopLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CLSFileOperation()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_LSFILEOPERATION)


BEGIN_COM_MAP(CLSFileOperation)
	COM_INTERFACE_ENTRY(ILSFileOperation)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
	STDMETHOD(FileCopy)(HWND hwnd, LPCWSTR pszSrc, LPCWSTR pszDst, BOOL bCopy);
	STDMETHOD(FileCopyEx)(LPARAM param);
	STDMETHOD(DropFileOperation)(HWND hwnd, HGLOBAL hdrop, DWORD eff, LPCWSTR pszDst, BOOL bSaveFolderHierarchy);
	STDMETHOD(DropFileOperationEx)(LPARAM param);
	STDMETHOD(MakeTree)(LPCWSTR pszDir);
	STDMETHOD(MakeTaskJob)(HWND hwnd, LPCWSTR pszSyncFile);
	STDMETHOD(MakeSymLinkUI)(HWND hwnd, LPCWSTR lpSymlinkBase, LPCWSTR lpTargetFileName);
	STDMETHOD(MakeSymLink)(HWND hwnd, LPCWSTR lpTarget, LPCWSTR lpSrc);
};

OBJECT_ENTRY_AUTO(__uuidof(LSFileOperation), CLSFileOperation)
