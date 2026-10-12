// AddPack.h : CAddPack の宣言

#pragma once
#include "resource.h"       // メイン シンボル

#include "lsshell_i.h"


// CAddPack

class ATL_NO_VTABLE CAddPack : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAddPack, &CLSID_AddPack>,
	public IDispatchImpl<IAddPack, &IID_IAddPack, &LIBID_lsshellLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IPersistFile,
	public IDropTarget
{
public:
	CAddPack()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_ADDPACK)


BEGIN_COM_MAP(CAddPack)
	COM_INTERFACE_ENTRY(IAddPack)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IPersistFile)
	COM_INTERFACE_ENTRY(IDropTarget)
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
	// IPersistFile
	STDMETHOD(GetClassID)(LPCLSID) { return S_FALSE; }
	STDMETHOD(IsDirty)() { return S_FALSE; }
	STDMETHOD(Save)(LPCOLESTR, BOOL) { return S_FALSE; }
	STDMETHOD(SaveCompleted)(LPCOLESTR) { return S_FALSE; }
	STDMETHOD(GetCurFile)(LPOLESTR FAR*) { return S_FALSE; }
	STDMETHOD(Load)(LPCOLESTR, DWORD);

	// IDropTarget
	STDMETHOD(DragEnter)(IDataObject*, DWORD, POINTL, DWORD*);
	STDMETHOD(DragOver)(DWORD, POINTL, DWORD*);
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop)(IDataObject*, DWORD, POINTL, DWORD*);

protected:
	int m_userlevel;
	TCHAR m_szFile[MAX_PATH * 2];
};

OBJECT_ENTRY_AUTO(__uuidof(AddPack), CAddPack)
