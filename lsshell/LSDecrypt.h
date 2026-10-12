// LSDecrypt.h : CLSDecrypt の宣言

#pragma once
#include "resource.h"       // メイン シンボル
#include "lsshell_i.h"
#include "foplib.h"

using namespace ATL;

////////////////////////////////////////////////////////////////////////
// CLSDecrypt

class ATL_NO_VTABLE CLSDecrypt :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CLSDecrypt, &CLSID_LSDecrypt>,
	public IDispatchImpl<ILSDecrypt, &IID_ILSDecrypt, &LIBID_lsshellLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IPersistFile,
	public IShellExtInit,
	public IInitializeWithFile,
	public IInitializeWithItem,
	public IQueryInfo,
	public IContextMenu
{
private:
	IDataObject* m_pDataObj;
	TCHAR m_path[MAX_PATH * 2];
	CSimpleMap<UINT, UINT> m_cmdmap;
	UINT m_idCmdFirst;
	CFopLib m_fop;
	TCHAR m_szFile[MAX_PATH];
	WIN32_FIND_DATA m_wfd;
	CString m_comment;

public:
	CLSDecrypt()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_DECRYPT)

BEGIN_COM_MAP(CLSDecrypt)
	COM_INTERFACE_ENTRY(ILSDecrypt)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IPersistFile)
	COM_INTERFACE_ENTRY(IInitializeWithFile)
	COM_INTERFACE_ENTRY(IInitializeWithItem)
	COM_INTERFACE_ENTRY(IQueryInfo)
	COM_INTERFACE_ENTRY(IContextMenu)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();
	void FinalRelease();

public:
	// IShellExtInit
	STDMETHOD(Initialize) (LPCITEMIDLIST, IDataObject*, HKEY);
	// IPersistFile
	STDMETHOD(Load)(LPCOLESTR wszFile, DWORD dwMode);
	STDMETHOD(GetClassID)(LPCLSID) { return E_NOTIMPL;	}
	STDMETHOD(IsDirty)(VOID) { return E_NOTIMPL; }
	STDMETHOD(Save)(LPCOLESTR, BOOL) { return E_NOTIMPL; }
	STDMETHOD(SaveCompleted)(LPCOLESTR) { return E_NOTIMPL; }
	STDMETHOD(GetCurFile)(LPOLESTR FAR*) { return E_NOTIMPL; }
	// IContextMenu
	STDMETHOD(GetCommandString)(UINT_PTR, UINT, UINT*, LPSTR, UINT);
	STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO);
	STDMETHOD(QueryContextMenu)(HMENU, UINT, UINT , UINT, UINT);
	// IInitializeWithFile
	STDMETHOD(Initialize)(LPCWSTR pszFilePath, DWORD grfMode);
	// IInitializeWithItem
	STDMETHOD(Initialize)(IShellItem *psi, DWORD grfMode);
	// IQueryInfo
	STDMETHOD(GetInfoFlags)(LPDWORD pdwFlags);
	STDMETHOD(GetInfoTip)(DWORD, LPWSTR*);

private:
	void InsertCommandMenu(HMENU hmenu, UINT iInsertAfter, UINT idCtxCommand, UINT idRealCommand, UINT idResource);
};

OBJECT_ENTRY_AUTO(__uuidof(LSDecrypt), CLSDecrypt)
