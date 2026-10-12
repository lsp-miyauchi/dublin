// CopyTo.h : CCopyTo の宣言

#pragma once
#include "resource.h"       // メイン シンボル
#include "lsshell_i.h"
#include "dropfiles.h"


// CCopyTo

class ATL_NO_VTABLE CCopyTo : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCopyTo, &CLSID_CopyTo>,
	public IDispatchImpl<ICopyTo, &IID_ICopyTo, &LIBID_lsshellLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IShellExtInit,
	public IContextMenu
{
private:
	CDropFiles m_df;
	int m_userlevel;
	TCHAR m_szPath[MAX_PATH * 2];
	CSimpleMap<UINT, UINT> m_cmdmap;
	UINT m_idCmdFirst;
	DWORD m_operation;

public:
	CCopyTo()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_COPYTO)


BEGIN_COM_MAP(CCopyTo)
	COM_INTERFACE_ENTRY(ICopyTo)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IContextMenu)
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
	// IShellExtInit
	STDMETHOD(Initialize) (LPCITEMIDLIST, LPDATAOBJECT, HKEY);
	// IContextMenu
	STDMETHOD(GetCommandString)(UINT_PTR, UINT, UINT*, LPSTR, UINT);
	STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO);
	STDMETHOD(QueryContextMenu)(HMENU, UINT, UINT , UINT, UINT);

private:
	void InsertCommandMenu(HMENU hmenu, UINT iInsertAfter, UINT idCommand, UINT idResource, UINT flags);
};

OBJECT_ENTRY_AUTO(__uuidof(CopyTo), CCopyTo)
