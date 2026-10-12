// IconProp.h : CIconProp �̐錾

#pragma once
#include <atlapp.h>
#include <atldlgs.h>

#include "lsshell_i.h"
#include "mainmodule.h"

#define WM_NOTIFY_HANDLER(cd, id, func) \
	if (uMsg == WM_NOTIFY && ((NMHDR*)lParam)->idFrom == id && ((NMHDR*)lParam)->code == cd) { \
		lResult = 0; \
		func((NMHDR*)lParam, &lResult); \
		return TRUE; \
	}


// CIconProp

struct __declspec(uuid("000214e8-0000-0000-c000-000000000046")) IShellExtInit;
struct __declspec(uuid("000214e9-0000-0000-c000-000000000046")) IShellPropSheetExt;

////////////////////////////////////////////////////////////////////////
// CIconProp

class CIconPropPage : public CPropertyPageImpl<CIconPropPage>
{
public:
	enum { IDD = IDD_ICONPROP };
	CString m_name;
protected:
	CLccSliderCtrl m_slider;
	CListViewCtrl m_list;
	CFopLib m_fop;
	HANDLE m_hIcm;

public:
	CIconPropPage() : m_hIcm(NULL)
	{ }
	virtual ~CIconPropPage()
	{
		if (m_hIcm) { m_fop.IcmClose(m_hIcm); }
	}
	void OnFinalMessage(HWND /*hWnd*/) { delete this; }

// Overrides
public:
	BOOL OnApply(void);

protected:
	BEGIN_MSG_MAP(CIconPropPage)
		CHAIN_MSG_MAP(CPropertyPageImpl<CIconPropPage>)
		ON_WM_INITDIALOG()
		ON_WM_HSCROLL()
		ON_BN_CLICKED(IDC_ICON_WRITE, OnExportIcons)
		ON_NOTIFY(NM_DBLCLK, IDC_ICON_LIST, OnNotifyDblClk)
		ON_NOTIFY(LVN_BEGINDRAG, IDC_ICON_LIST, OnNotifyBeginDrag)
		ON_BN_CLICKED(IDC_ICON_DETAIL, OnShowIconDetail)
	END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	inline void OnNotifyDblClk(NMHDR* phdr, LRESULT* pResult) { OnShowIconDetail(); }
	void OnNotifyBeginDrag(NMHDR* pNmhdr, LRESULT* pResult);
	void OnInit(WPARAM wParam, LPARAM lParam);
	void OnHScroll(UINT nSBCode, UINT nPos, HWND hctl);
	void ChangeIconView(int dx);
	void OnExportIcons();
	void OnShowIconDetail();

	void EnableButtons(BOOL enable);
};


////////////////////////////////////////////////////////////////////////
// CIconProp

class ATL_NO_VTABLE CIconProp : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CIconProp, &CLSID_IconProp>,
	public IDispatchImpl<IIconProp, &IID_IIconProp, &LIBID_lsshellLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IShellExtInit,
	public IShellPropSheetExt
{
private:
	CString m_file;

public:
	CIconProp()
	{
		m_file = _T("");
	}

DECLARE_REGISTRY_RESOURCEID(IDR_EXIFPROP)


BEGIN_COM_MAP(CIconProp)
	COM_INTERFACE_ENTRY(IIconProp)
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

OBJECT_ENTRY_AUTO(__uuidof(IconProp), CIconProp)
