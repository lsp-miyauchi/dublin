// ExifProp.h : CExifProp �̐錾

#pragma once
#include <atlapp.h>
#include <atldlgs.h>

#include "lsshell_i.h"
#include "mainmodule.h"

#include "..\lsimg\lsimg.h"
#include "..\lspexif\lspexifapi.h"


#define WM_NOTIFY_HANDLER(cd, id, func) \
	if (uMsg == WM_NOTIFY && ((NMHDR*)lParam)->idFrom == id && ((NMHDR*)lParam)->code == cd) { \
		lResult = 0; \
		func((NMHDR*)lParam, &lResult); \
		return TRUE; \
	}


// CExifProp

struct __declspec(uuid("000214e8-0000-0000-c000-000000000046")) IShellExtInit;
struct __declspec(uuid("000214e9-0000-0000-c000-000000000046")) IShellPropSheetExt;

////////////////////////////////////////////////////////////////////////
// CExifProp

class CExifPropPage : public CPropertyPageImpl<CExifPropPage>
{
public:
	enum { IDD = IDD_EXIFPROP };

	CExifPropPage() { }
	virtual ~CExifPropPage() { }
	void OnFinalMessage(HWND /*hWnd*/) { delete this; }

// Overrides
public:
	BOOL OnApply(void);

// Class members
public:
	CString	m_file;

protected:
	CListViewCtrl m_list;
	LVGROUP m_group;
	LV_ITEM m_item;

public:
	BEGIN_MSG_MAP(CExifPropPage)
		CHAIN_MSG_MAP(CPropertyPageImpl<CExifPropPage>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		WM_NOTIFY_HANDLER(NM_CUSTOMDRAW, IDC_LIST, OnNotifyCustomDraw)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnNotifyCustomDraw(NMHDR* phdr, LRESULT* pResult);

	int AddItemOrGroup(bool group, LPCTSTR pszTitle, LPCTSTR pszText);
	static int CALLBACK ExifProc(int type, LPCTSTR pszTitle, LPCTSTR pszText, LPARAM param);
};


////////////////////////////////////////////////////////////////////////
// CExifProp

class ATL_NO_VTABLE CExifProp : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CExifProp, &CLSID_ExifProp>,
	public IDispatchImpl<IExifProp, &IID_IExifProp, &LIBID_lsshellLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IShellExtInit,
	public IShellPropSheetExt
{
private:
	CString m_file;

public:
	CExifProp()
	{
		m_file = _T("");
	}

DECLARE_REGISTRY_RESOURCEID(IDR_EXIFPROP)


BEGIN_COM_MAP(CExifProp)
	COM_INTERFACE_ENTRY(IExifProp)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IShellPropSheetExt)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease()
	{
	}

	///////////////////////////////////////////////////////////
	// IShellExtInit
	//
	STDMETHOD(Initialize)(LPCITEMIDLIST, LPDATAOBJECT, HKEY);

	///////////////////////////////////////////////////////////
	// IShellPropSheetExt
	STDMETHOD(AddPages)(LPFNADDPROPSHEETPAGE, LPARAM);
    STDMETHOD(ReplacePage)(UINT, LPFNADDPROPSHEETPAGE, LPARAM) { return E_NOTIMPL; }

public:
	int m_userlevel;
};

OBJECT_ENTRY_AUTO(__uuidof(ExifProp), CExifProp)
