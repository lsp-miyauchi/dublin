// Ctx.h : CCtx の宣言

#pragma once
#include "resource.h"       // メイン シンボル
#include "lsshell_i.h"
#include "dropfiles.h"

#define STS_ARCPACK					0x00000001
#define STS_ARCUNPACK				0x00000002
#define STS_ARCCHECK				0x00000004
#define STS_BDUMP					0x00000008
#define STS_DIV						0x00000010
#define STS_SERIESNM				0x00000020
#define STS_REPNAMES				0x00000040
#define STS_CHGCAPS					0x00000080
#define STS_DFLUSH					0x00000100
#define STS_SYNC					0x00000400
#define STS_MERGE					0x00000800
#define STS_ATTRIB					0x00001000
#define STS_FILTERDEL				0x00002000
#define STS_FSIZE					0x00004000
#define STS_DRVINFO					0x00008000
#define STS_ICONINFO				0x00010000
#define STS_FILEINFO				0x00020000
#define STS_PATHCLIP				0x00040000		// パス名のコピー
#define STS_RENAME					0x00080000
#define STS_CRYPT					0x00100000
#define STS_REPARSE					0x00200000
#define STS_CALCHASH				0x00400000
#define STS_SHREDDER				0x00800000		// シュレッダ
#define STS_ICONPICKER				0x01000000		// アイコン抽出
#define STS_SUBMENU					0x80000000		// サブメニュー化

// CCtx

class ATL_NO_VTABLE CCtx : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCtx, &CLSID_Ctx>,
	public IDispatchImpl<ICtx, &IID_ICtx, &LIBID_lsshellLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IShellExtInit,
	public IContextMenu3
{
public:
	CCtx()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_CTX)


BEGIN_COM_MAP(CCtx)
	COM_INTERFACE_ENTRY(ICtx)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IContextMenu)
//	COM_INTERFACE_ENTRY(IContextMenu2)
	COM_INTERFACE_ENTRY(IContextMenu3)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();
	void FinalRelease();

public:
	// IShellExtInit
	STDMETHOD(Initialize) (LPCITEMIDLIST, LPDATAOBJECT, HKEY);

	// IContextMenu
	STDMETHOD(GetCommandString)(UINT_PTR, UINT, UINT*, LPSTR, UINT);
	STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO);
	STDMETHOD(QueryContextMenu)(HMENU, UINT, UINT , UINT, UINT);

	// IContextMenu2
	STDMETHOD(HandleMenuMsg)(UINT, WPARAM, LPARAM);

	// IContextMenu3
	STDMETHOD(HandleMenuMsg2)(UINT, WPARAM, LPARAM, LRESULT *);

	// ICtx
	STDMETHOD(Setup)(HWND hwnd);

protected:
	void MeasureItem(LPMEASUREITEMSTRUCT);
	void DrawMenuItem(LPDRAWITEMSTRUCT);
	void InsertCommandMenu(HMENU hmenu, UINT iInsertAfter, UINT idBase, UINT idCommand, UINT flags, UINT idImage = 0);
	void MenuChar(TCHAR ch, UINT flags, HMENU hmenu, LRESULT* plResult);

protected:
	CDropFiles m_df;
	UINT m_idCmdFirst;
	int m_userlevel;
	TCHAR m_szFile[MAX_PATH * 2];
	UINT m_flags;
	CSimpleMap<UINT, UINT> m_arrCommand;
	CSimpleMap<TCHAR, UINT> m_arrAccessKey;
};

OBJECT_ENTRY_AUTO(__uuidof(Ctx), CCtx)
