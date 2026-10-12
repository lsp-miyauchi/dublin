// PackTo.h : CPackTo の宣言

#pragma once
#include "resource.h"       // メイン シンボル
#include "lsshell_i.h"
#include "dropfiles.h"


// CPackTo

class ATL_NO_VTABLE CPackTo : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CPackTo, &CLSID_PackTo>,
	public IDispatchImpl<IPackTo, &IID_IPackTo, &LIBID_lsshellLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IShellExtInit,
	public IContextMenu
{
public:
	CPackTo()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_PACKTO)


BEGIN_COM_MAP(CPackTo)
	COM_INTERFACE_ENTRY(IPackTo)
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

protected:
	CDropFiles m_df;
	int m_userlevel;
	TCHAR m_szPath[MAX_PATH * 2];
};

OBJECT_ENTRY_AUTO(__uuidof(PackTo), CPackTo)
