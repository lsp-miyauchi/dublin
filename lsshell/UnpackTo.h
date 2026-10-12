// UnpackTo.h : CUnpackTo の宣言

#pragma once
#include "resource.h"       // メイン シンボル
#include "lsshell_i.h"
#include "dropfiles.h"

// CUnpackTo

class ATL_NO_VTABLE CUnpackTo : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CUnpackTo, &CLSID_UnpackTo>,
	public IDispatchImpl<IUnpackTo, &IID_IUnpackTo, &LIBID_lsshellLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IShellExtInit,
	public IContextMenu
{
public:
	CUnpackTo()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_UNPACKTO)


BEGIN_COM_MAP(CUnpackTo)
	COM_INTERFACE_ENTRY(IUnpackTo)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IContextMenu)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		m_arrCommand.RemoveAll();
		m_flags = 0;
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
	UINT m_idCmdFirst;
	int m_userlevel;
	TCHAR m_szFile[MAX_PATH * 2];
	TCHAR m_szPath[MAX_PATH * 2];
	DWORD m_flags;
	CSimpleMap<UINT, UINT> m_arrCommand;
};

OBJECT_ENTRY_AUTO(__uuidof(UnpackTo), CUnpackTo)
