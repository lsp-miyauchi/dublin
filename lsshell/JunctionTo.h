// JunctionTo.h : CJunctionTo の宣言

#pragma once
#include "resource.h"       // メイン シンボル
#include "lsshell_i.h"
#include "dropfiles.h"


// CJunctionTo

class ATL_NO_VTABLE CJunctionTo : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CJunctionTo, &CLSID_JunctionTo>,
	public IDispatchImpl<IJunctionTo, &IID_IJunctionTo, &LIBID_lsshellLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IShellExtInit,
	public IContextMenu
{
public:
	CJunctionTo()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_JUNCTIONTO)


BEGIN_COM_MAP(CJunctionTo)
	COM_INTERFACE_ENTRY(IJunctionTo)
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
	CString m_strName;
	CSimpleArray<UINT> m_commands;
};

OBJECT_ENTRY_AUTO(__uuidof(JunctionTo), CJunctionTo)
