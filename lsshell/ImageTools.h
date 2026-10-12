// ImageTools.h : CImageTools の宣言

#pragma once
#include "resource.h"       // メイン シンボル
#include "lsshell_i.h"
#include "dropfiles.h"


// CImageTools

class ATL_NO_VTABLE CImageTools : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CImageTools, &CLSID_ImageTools>,
	public IDispatchImpl<IImageTools, &IID_IImageTools, &LIBID_lsshellLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IShellExtInit,
	public IContextMenu
{
protected:
	CDropFiles m_df;
	UINT m_idCmdFirst;
	int m_userlevel;
	DWORD m_flags;
	int m_numJpeg;
	int m_numPng;
	int m_numTiff;
	int m_numWebp;
	int m_numAvif;
	CSimpleMap<UINT, UINT> m_arrCommand;
	bool m_exif:1;
	bool m_imageoperation:1;
	bool m_clipboard:1;

public:
	CImageTools() {}

DECLARE_REGISTRY_RESOURCEID(IDR_IMAGETOOLS)


BEGIN_COM_MAP(CImageTools)
	COM_INTERFACE_ENTRY(IImageTools)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IContextMenu)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		m_exif = false;
		m_imageoperation = false;
		m_clipboard = false;
		m_numJpeg = 0;
		m_numPng = 0;
		m_numTiff = 0;
		m_numWebp = 0;
		m_numAvif = 0;
		m_arrCommand.RemoveAll();
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
	void InsertCommandMenu(HMENU hmenu, UINT iInsertAfter, UINT idBase, UINT idCommand, UINT flags);
	void AppendCommandMenu(HMENU hmenu, UINT idBase, UINT idCommand, UINT flags);
};

OBJECT_ENTRY_AUTO(__uuidof(ImageTools), CImageTools)
