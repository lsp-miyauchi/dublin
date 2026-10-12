#ifndef __PROPSHEET_H__
#define __PROPSHEET_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLWIN_H__
	#error propsheet.h requires atlwin.h to be included first
#endif

#include <shlobj.h>


/////////////////////////////////////////////////////////////////////////////
// CPropertySheetWindow - client side for a property sheet

class CPropertySheetWindow : public CWindow
{
public:
// Constructors
	CPropertySheetWindow(HWND hWnd = NULL) : CWindow(hWnd) { }

	CPropertySheetWindow& operator=(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

// Attributes
	int GetPageCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HWND hWndTabCtrl = GetTabControl();
		ATLASSERT(hWndTabCtrl != NULL);
		return (int)::SendMessage(hWndTabCtrl, TCM_GETITEMCOUNT, 0, 0L);
	}
	HWND GetActivePage() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HWND)::SendMessage(m_hWnd, PSM_GETCURRENTPAGEHWND, 0, 0L);
	}
	int GetActiveIndex() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		HWND hWndTabCtrl = GetTabControl();
		ATLASSERT(hWndTabCtrl != NULL);
		return (int)::SendMessage(hWndTabCtrl, TCM_GETCURSEL, 0, 0L);
	}
	BOOL SetActivePage(int nPageIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, PSM_SETCURSEL, nPageIndex, 0L);
	}
	BOOL SetActivePage(HPROPSHEETPAGE hPage)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(hPage != NULL);
		return (BOOL)::SendMessage(m_hWnd, PSM_SETCURSEL, 0, (LPARAM)hPage);
	}
	BOOL SetActivePageByID(int nPageID)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, PSM_SETCURSELID, 0, nPageID);
	}
	void SetTitle(LPCTSTR lpszText, UINT nStyle = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT((nStyle & ~PSH_PROPTITLE) == 0); // only PSH_PROPTITLE is valid
		ATLASSERT(lpszText != NULL);
		::SendMessage(m_hWnd, PSM_SETTITLE, nStyle, (LPARAM)lpszText);
	}
	HWND GetTabControl() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HWND)::SendMessage(m_hWnd, PSM_GETTABCONTROL, 0, 0L);
	}
	void SetFinishText(LPCTSTR lpszText)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, PSM_SETFINISHTEXT, 0, (LPARAM)lpszText);
	}
	void SetWizardButtons(DWORD dwFlags)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::PostMessage(m_hWnd, PSM_SETWIZBUTTONS, 0, dwFlags);
	}

// Operations
	void AddPage(HPROPSHEETPAGE hPage)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(hPage != NULL);
		::SendMessage(m_hWnd, PSM_ADDPAGE, 0, (LPARAM)hPage);
	}
	BOOL AddPage(LPCPROPSHEETPAGE pPage)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(pPage != NULL);
		HPROPSHEETPAGE hPage = ::CreatePropertySheetPage(pPage);
		if(hPage == NULL)
			return FALSE;
		::SendMessage(m_hWnd, PSM_ADDPAGE, 0, (LPARAM)hPage);
		return TRUE;
	}
	void RemovePage(int nPageIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, PSM_REMOVEPAGE, nPageIndex, 0L);
	}
	void RemovePage(HPROPSHEETPAGE hPage)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(hPage != NULL);
		::SendMessage(m_hWnd, PSM_REMOVEPAGE, 0, (LPARAM)hPage);
	}
	BOOL PressButton(int nButton)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, PSM_PRESSBUTTON, nButton, 0L);
	}
	BOOL Apply()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, PSM_APPLY, 0, 0L);
	}
	void CancelToClose()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, PSM_CANCELTOCLOSE, 0, 0L);
	}
	void SetModified(HWND hWndPage, BOOL bChanged = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(::IsWindow(hWndPage));
		UINT uMsg = bChanged ? PSM_CHANGED : PSM_UNCHANGED;
		::SendMessage(m_hWnd, uMsg, (WPARAM)hWndPage, 0L);
	}
	LRESULT QuerySiblings(WPARAM wParam, LPARAM lParam)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return ::SendMessage(m_hWnd, PSM_QUERYSIBLINGS, wParam, lParam);
	}
	void RebootSystem()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, PSM_REBOOTSYSTEM, 0, 0L);
	}
	void RestartWindows()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, PSM_RESTARTWINDOWS, 0, 0L);
	}
	BOOL IsDialogMessage(LPMSG lpMsg)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, PSM_ISDIALOGMESSAGE, 0, (LPARAM)lpMsg);
	}

#if (_WIN32_IE >= 0x0500)
	int HwndToIndex(HWND hWnd) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PSM_HWNDTOINDEX, (WPARAM)hWnd, 0L);
	}
	HWND IndexToHwnd(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HWND)::SendMessage(m_hWnd, PSM_INDEXTOHWND, nIndex, 0L);
	}
	int PageToIndex(HPROPSHEETPAGE hPage) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PSM_PAGETOINDEX, 0, (LPARAM)hPage);
	}
	HPROPSHEETPAGE IndexToPage(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HPROPSHEETPAGE)::SendMessage(m_hWnd, PSM_INDEXTOPAGE, nIndex, 0L);
	}
	int IdToIndex(int nID) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PSM_IDTOINDEX, 0, nID);
	}
	int IndexToId(int nIndex) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PSM_INDEXTOID, nIndex, 0L);
	}
	int GetResult() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, PSM_GETRESULT, 0, 0L);
	}
	BOOL RecalcPageSizes()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, PSM_RECALCPAGESIZES, 0, 0L);
	}
#endif //(_WIN32_IE >= 0x0500)

// Implementation - override to prevent usage
	HWND Create(LPCTSTR, HWND, _U_RECT = NULL, LPCTSTR = NULL, DWORD = 0, DWORD = 0, _U_MENUorID = 0U, LPVOID = NULL)
	{
		ATLASSERT(FALSE);
		return NULL;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CPropertySheetImpl - implements a property sheet

#if (_MSC_VER >= 1200)
typedef HPROPSHEETPAGE	_HPROPSHEETPAGE_TYPE;
#else
// we use void* here instead of HPROPSHEETPAGE becuase HPROPSHEETPAGE
// is a _PSP*, but _PSP is not defined properly
typedef void*	_HPROPSHEETPAGE_TYPE;
#endif

template <class T, class TBase = CPropertySheetWindow>
class ATL_NO_VTABLE CPropertySheetImpl : public CWindowImplBaseT< TBase >
{
public:
	PROPSHEETHEADER m_psh;
	CSimpleArray<_HPROPSHEETPAGE_TYPE> m_arrPages;

// Construction/Destruction
	CPropertySheetImpl(_U_STRINGorID title = (LPCTSTR)NULL, UINT uStartPage = 0, HWND hWndParent = NULL)
	{
		memset(&m_psh, 0, sizeof(PROPSHEETHEADER));
		m_psh.dwSize = sizeof(PROPSHEETHEADER);
		m_psh.dwFlags = PSH_USECALLBACK;
		m_psh.hInstance = ATL::_AtlBaseModule.GetResourceInstance();
		m_psh.phpage = NULL;	// will be set later
		m_psh.nPages = 0;	// will be set later
		m_psh.pszCaption = title.m_lpstr;
		m_psh.nStartPage = uStartPage;
		m_psh.hwndParent = hWndParent;	// if NULL, will be set in DoModal/Create
		m_psh.pfnCallback = T::PropSheetCallback;
	}

	~CPropertySheetImpl()
	{
		if(m_arrPages.GetSize() > 0)	// sheet never created, destroy all pages
		{
			for(int i = 0; i < m_arrPages.GetSize(); i++)
				::DestroyPropertySheetPage((HPROPSHEETPAGE)m_arrPages[i]);
		}
	}

	static int CALLBACK PropSheetCallback(HWND hWnd, UINT uMsg, LPARAM)
	{
		if(uMsg == PSCB_INITIALIZED)
		{
			ATLASSERT(hWnd != NULL);
			T* pT = (T*)_AtlWinModule.ExtractCreateWndData();
			// subclass the sheet window
			pT->SubclassWindow(hWnd);
			// remove page handles array
			pT->_CleanUpPages();
		}

		return 0;
	}

	HWND Create(HWND hWndParent = NULL)
	{
		ATLASSERT(m_hWnd == NULL);

		m_psh.dwFlags |= PSH_MODELESS;
		if(m_psh.hwndParent == NULL)
			m_psh.hwndParent = hWndParent;
		m_psh.phpage = (HPROPSHEETPAGE*)m_arrPages.GetData();
		m_psh.nPages = m_arrPages.GetSize();

		T* pT = static_cast<T*>(this);
		_Module.AddCreateWndData(&m_thunk.cd, pT);

		HWND hWnd = (HWND)::PropertySheet(&m_psh);
		_CleanUpPages();	// ensure clean-up, required if call failed

		ATLASSERT(m_hWnd == hWnd);

		return hWnd;
	}

	INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
	{
		ATLASSERT(m_hWnd == NULL);

		m_psh.dwFlags &= ~PSH_MODELESS;
		if(m_psh.hwndParent == NULL)
			m_psh.hwndParent = hWndParent;
		m_psh.phpage = (HPROPSHEETPAGE*)m_arrPages.GetData();
		m_psh.nPages = m_arrPages.GetSize();

		T* pT = static_cast<T*>(this);
		_AtlWinModule.AddCreateWndData(&m_thunk.cd, pT);

		INT_PTR nRet = ::PropertySheet(&m_psh);
		_CleanUpPages();	// ensure clean-up, required if call failed

		return nRet;
	}

	// implementation helper - clean up pages array
	void _CleanUpPages()
	{
		m_psh.nPages = 0;
		m_psh.phpage = NULL;
		m_arrPages.RemoveAll();
	}

// Attributes (extended overrides of client class methods)
// These now can be called before the sheet is created
// Note: Calling these after the sheet is created gives unpredictable results
	int GetPageCount() const
	{
		if(m_hWnd == NULL)	// not created yet
			return m_arrPages.GetSize();
		return TBase::GetPageCount();
	}
	int GetActiveIndex() const
	{
		if(m_hWnd == NULL)	// not created yet
			return m_psh.nStartPage;
		return TBase::GetActiveIndex();
	}
	HPROPSHEETPAGE GetPage(int nPageIndex) const
	{
		ATLASSERT(m_hWnd == NULL);	// can't do this after it's created
		return (HPROPSHEETPAGE)m_arrPages[nPageIndex];
	}
	int GetPageIndex(HPROPSHEETPAGE hPage) const
	{
		ATLASSERT(m_hWnd == NULL);	// can't do this after it's created
		return m_arrPages.Find((_HPROPSHEETPAGE_TYPE&)hPage);
	}
	BOOL SetActivePage(int nPageIndex)
	{
		if(m_hWnd == NULL)	// not created yet
		{
			ATLASSERT(nPageIndex >= 0 && nPageIndex < m_arrPages.GetSize());
			m_psh.nStartPage = nPageIndex;
			return TRUE;
		}
		return TBase::SetActivePage(nPageIndex);
	}
	BOOL SetActivePage(HPROPSHEETPAGE hPage)
	{
		ATLASSERT(hPage != NULL);
		if (m_hWnd == NULL)	// not created yet
		{
			int nPageIndex = GetPageIndex(hPage);
			if(nPageIndex == -1)
				return FALSE;

			return SetActivePage(nPageIndex);
		}
		return TBase::SetActivePage(hPage);

	}
	void SetTitle(LPCTSTR lpszText, UINT nStyle = 0)
	{
		ATLASSERT((nStyle & ~PSH_PROPTITLE) == 0); // only PSH_PROPTITLE is valid
		ATLASSERT(lpszText != NULL);

		if(m_hWnd == NULL)
		{
			// set internal state
			m_psh.pszCaption = lpszText;	// must exist until sheet is created
			m_psh.dwFlags &= ~PSH_PROPTITLE;
			m_psh.dwFlags |= nStyle;
		}
		else
		{
			// set external state
			TBase::SetTitle(lpszText, nStyle);
		}
	}
	void SetWizardMode()
	{
		m_psh.dwFlags |= PSH_WIZARD;
	}
	void EnableHelp()
	{
		m_psh.dwFlags |= PSH_HASHELP;
	}

// Operations
	BOOL AddPage(HPROPSHEETPAGE hPage)
	{
		ATLASSERT(hPage != NULL);
		BOOL bRet = TRUE;
		if(m_hWnd != NULL)
			TBase::AddPage(hPage);
		else	// sheet not created yet, use internal data
			bRet = m_arrPages.Add((_HPROPSHEETPAGE_TYPE&)hPage);
		return bRet;
	}
	BOOL AddPage(LPCPROPSHEETPAGE pPage)
	{
		ATLASSERT(pPage != NULL);
		HPROPSHEETPAGE hPage = ::CreatePropertySheetPage(pPage);
		if(hPage == NULL)
			return FALSE;
		BOOL bRet = AddPage(hPage);
		if(!bRet)
			::DestroyPropertySheetPage(hPage);
		return bRet;
	}
	BOOL RemovePage(HPROPSHEETPAGE hPage)
	{
		ATLASSERT(hPage != NULL);
		if (m_hWnd == NULL)		// not created yet
		{
			int nPage = GetPageIndex(hPage);
			if(nPage == -1)
				return FALSE;
			return RemovePage(nPage);
		}
		TBase::RemovePage(hPage);
		return TRUE;

	}
	BOOL RemovePage(int nPageIndex)
	{
		BOOL bRet = TRUE;
		if(m_hWnd != NULL)
			TBase::RemovePage(nPageIndex);
		else	// sheet not created yet, use internal data
			bRet = m_arrPages.RemoveAt(nPageIndex);
		return bRet;
	}

#if (_WIN32_IE >= 0x0400)
	void SetHeader(LPCTSTR szbmHeader)
	{
		ATLASSERT(m_hWnd == NULL);	// can't do this after it's created

		m_psh.dwFlags &= ~PSH_WIZARD;
		m_psh.dwFlags |= (PSH_HEADER | PSH_WIZARD97);
		m_psh.pszbmHeader = szbmHeader;
	}

	void SetHeader(HBITMAP hbmHeader)
	{
		ATLASSERT(m_hWnd == NULL);	// can't do this after it's created

		m_psh.dwFlags &= ~PSH_WIZARD;
		m_psh.dwFlags |= (PSH_HEADER | PSH_USEHBMHEADER | PSH_WIZARD97);
		m_psh.hbmHeader = hbmHeader;
	}

	void SetWatermark(LPCTSTR szbmWatermark, HPALETTE hplWatermark = NULL)
	{
		ATLASSERT(m_hWnd == NULL);	// can't do this after it's created

		m_psh.dwFlags &= ~PSH_WIZARD;
		m_psh.dwFlags |= PSH_WATERMARK | PSH_WIZARD97;
		m_psh.pszbmWatermark = szbmWatermark;

		if (hplWatermark != NULL)
		{
			m_psh.dwFlags |= PSH_USEHPLWATERMARK;
			m_psh.hplWatermark = hplWatermark;
		}
	}

	void SetWatermark(HBITMAP hbmWatermark, HPALETTE hplWatermark = NULL)
	{
		ATLASSERT(m_hWnd == NULL);	// can't do this after it's created

		m_psh.dwFlags &= ~PSH_WIZARD;
		m_psh.dwFlags |= (PSH_WATERMARK | PSH_USEHBMWATERMARK | PSH_WIZARD97);
		m_psh.hbmWatermark = hbmWatermark;

		if (hplWatermark != NULL)
		{
			m_psh.dwFlags |= PSH_USEHPLWATERMARK;
			m_psh.hplWatermark = hplWatermark;
		}
	}

	void StretchWatermark(bool bStretchWatermark)
	{
		ATLASSERT(m_hWnd == NULL);	// can't do this after it's created
		if (bStretchWatermark)
			m_psh.dwFlags |= PSH_STRETCHWATERMARK;
		else
			m_psh.dwFlags &= ~PSH_STRETCHWATERMARK;
	}
#endif

// Message map and handlers
	BEGIN_MSG_MAP(CPropertySheetImpl)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
	END_MSG_MAP()

	LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
		if(HIWORD(wParam) == BN_CLICKED && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) &&
		   ((m_psh.dwFlags & PSH_MODELESS) != 0) && (GetActivePage() == NULL))
			DestroyWindow();
		return lRet;
	}

	LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if(((m_psh.dwFlags & PSH_MODELESS) == PSH_MODELESS) && ((wParam & 0xFFF0) == SC_CLOSE))
			SendMessage(WM_CLOSE);
		else
			bHandled = FALSE;
		return 0;
	}
};

// for non-customized sheets
class CPropertySheet : public CPropertySheetImpl<CPropertySheet>
{
public:
	CPropertySheet(_U_STRINGorID title = (LPCTSTR)NULL, UINT uStartPage = 0, HWND hWndParent = NULL)
		: CPropertySheetImpl<CPropertySheet>(title, uStartPage, hWndParent)
	{ }

	BEGIN_MSG_MAP(CPropertySheet)
		MESSAGE_HANDLER(WM_COMMAND, CPropertySheetImpl<CPropertySheet>::OnCommand)
	END_MSG_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CPropertyPageWindow - client side for a property page

class CPropertyPageWindow : public CWindow
{
public:
// Constructors
	CPropertyPageWindow(HWND hWnd = NULL) : CWindow(hWnd) { }

	CPropertyPageWindow& operator=(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

// Attributes
	CPropertySheetWindow GetPropertySheet() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CPropertySheetWindow(GetParent());
	}

// Operations
	BOOL Apply()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		return GetPropertySheet().Apply();
	}
	void CancelToClose()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		GetPropertySheet().CancelToClose();
	}
	void SetModified(BOOL bChanged = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		GetPropertySheet().SetModified(m_hWnd, bChanged);
	}
	LRESULT QuerySiblings(WPARAM wParam, LPARAM lParam)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		return GetPropertySheet().QuerySiblings(wParam, lParam);
	}
	void RebootSystem()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		GetPropertySheet().RebootSystem();
	}
	void RestartWindows()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		GetPropertySheet().RestartWindows();
	}
	void SetWizardButtons(DWORD dwFlags)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		ATLASSERT(GetParent() != NULL);
		GetPropertySheet().SetWizardButtons(dwFlags);
	}

// Implementation - overrides to prevent usage
	HWND Create(LPCTSTR, HWND, _U_RECT = NULL, LPCTSTR = NULL, DWORD = 0, DWORD = 0, _U_MENUorID = 0U, LPVOID = NULL)
	{
		ATLASSERT(FALSE);
		return NULL;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CPropertyPageImpl - implements a property page

template <class T, class TBase = CPropertyPageWindow>
class ATL_NO_VTABLE CPropertyPageImpl : public CDialogImplBaseT< TBase >
{
public:
	PROPSHEETPAGE m_psp;

	operator PROPSHEETPAGE*() { return &m_psp; }

// Construction
	CPropertyPageImpl(_U_STRINGorID title = (LPCTSTR)NULL)
	{
		// initialize PROPSHEETPAGE struct
		memset(&m_psp, 0, sizeof(PROPSHEETPAGE));
		m_psp.dwSize = sizeof(PROPSHEETPAGE);
		m_psp.dwFlags = PSP_USECALLBACK;
		m_psp.hInstance = ATL::_AtlBaseModule.GetResourceInstance();
		T* pT = static_cast<T*>(this);
		m_psp.pszTemplate = MAKEINTRESOURCE(pT->IDD);
		m_psp.pfnDlgProc = (DLGPROC)T::StartDialogProc;
		m_psp.pfnCallback = T::PropPageCallback;
		m_psp.lParam = (LPARAM)pT;

		if(title.m_lpstr != NULL)
			SetTitle(title);
	}

	static UINT CALLBACK PropPageCallback(HWND hWnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
	{
		hWnd;	// avoid level 4 warning
		if(uMsg == PSPCB_CREATE)
		{
			ATLASSERT(hWnd == NULL);
			CDialogImplBaseT< TBase >* pPage = (CDialogImplBaseT< TBase >*)(T*)ppsp->lParam;
			_AtlWinModule.AddCreateWndData(&pPage->m_thunk.cd, pPage);
		}

		return 1;
	}

	HPROPSHEETPAGE Create()
	{
		return ::CreatePropertySheetPage(&m_psp);
	}

// Attributes
	void SetTitle(_U_STRINGorID title)
	{
		m_psp.pszTitle = title.m_lpstr;
		m_psp.dwFlags |= PSP_USETITLE;
	}

#if (_WIN32_IE >= 0x0500)
	void SetHeaderTitle(LPCTSTR lpstrHeaderTitle)
	{
		ATLASSERT(m_hWnd == NULL);	// can't do this after it's created
		m_psp.dwFlags |= PSP_USEHEADERTITLE;
		m_psp.pszHeaderTitle = lpstrHeaderTitle;
	}
	void SetHeaderSubTitle(LPCTSTR lpstrHeaderSubTitle)
	{
		ATLASSERT(m_hWnd == NULL);	// can't do this after it's created
		m_psp.dwFlags |= PSP_USEHEADERSUBTITLE;
		m_psp.pszHeaderSubTitle = lpstrHeaderSubTitle;
	}
#endif //(_WIN32_IE >= 0x0500)

// Operations
	void EnableHelp()
	{
		m_psp.dwFlags |= PSP_HASHELP;
	}

// Message map and handlers
	BEGIN_MSG_MAP(CPropertyPageImpl)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
	END_MSG_MAP()

	// NOTE: Define _WTL_NEW_PAGE_NOTIFY_HANDLERS to use new notification
	// handlers that return direct values without any restrictions
	LRESULT OnNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		NMHDR* pNMHDR = (NMHDR*)lParam;

		// don't handle messages not from the page/sheet itself
		if(pNMHDR->hwndFrom != m_hWnd && pNMHDR->hwndFrom != ::GetParent(m_hWnd))
		{
			bHandled = FALSE;
			return 1;
		}

		T* pT = static_cast<T*>(this);
		LRESULT lResult = 0;
		switch(pNMHDR->code)
		{
#ifdef _WTL_NEW_PAGE_NOTIFY_HANDLERS
		case PSN_SETACTIVE:
			lResult = pT->OnSetActive();
			break;
		case PSN_KILLACTIVE:
			lResult = pT->OnKillActive();
			break;
		case PSN_APPLY:
			lResult = pT->OnApply();
			break;
		case PSN_RESET:
			pT->OnReset();
			break;
		case PSN_QUERYCANCEL:
			lResult = pT->OnQueryCancel();
			break;
		case PSN_WIZNEXT:
			lResult = pT->OnWizardNext();
			break;
		case PSN_WIZBACK:
			lResult = pT->OnWizardBack();
			break;
		case PSN_WIZFINISH:
			lResult = pT->OnWizardFinish();
			break;
		case PSN_HELP:
			pT->OnHelp();
			break;
#if (_WIN32_IE >= 0x0400)
		case PSN_GETOBJECT:
			if(!pT->OnGetObject((LPNMOBJECTNOTIFY)lParam))
				bHandled = FALSE;
			break;
#endif //(_WIN32_IE >= 0x0400)
#if (_WIN32_IE >= 0x0500)
		case PSN_TRANSLATEACCELERATOR:
			{
				LPPSHNOTIFY lpPSHNotify = (LPPSHNOTIFY)lParam;
				lResult = pT->OnTranslateAccelerator((LPMSG)lpPSHNotify->lParam);
			}
			break;
		case PSN_QUERYINITIALFOCUS:
			{
				LPPSHNOTIFY lpPSHNotify = (LPPSHNOTIFY)lParam;
				lResult = (LRESULT)pT->OnQueryInitialFocus((HWND)lpPSHNotify->lParam);
			}
			break;
#endif //(_WIN32_IE >= 0x0500)

#else //!_WTL_NEW_PAGE_NOTIFY_HANDLERS
		case PSN_SETACTIVE:
			lResult = pT->OnSetActive() ? 0 : -1;
			break;
		case PSN_KILLACTIVE:
			lResult = !pT->OnKillActive();
			break;
		case PSN_APPLY:
			lResult = pT->OnApply() ? PSNRET_NOERROR : PSNRET_INVALID_NOCHANGEPAGE;
			break;
		case PSN_RESET:
			pT->OnReset();
			break;
		case PSN_QUERYCANCEL:
			lResult = !pT->OnQueryCancel();
			break;
		case PSN_WIZNEXT:
			lResult = pT->OnWizardNext();
			break;
		case PSN_WIZBACK:
			lResult = pT->OnWizardBack();
			break;
		case PSN_WIZFINISH:
			lResult = !pT->OnWizardFinish();
			break;
		case PSN_HELP:
			pT->OnHelp();
			break;
#if (_WIN32_IE >= 0x0400)
		case PSN_GETOBJECT:
			if(!pT->OnGetObject((LPNMOBJECTNOTIFY)lParam))
				bHandled = FALSE;
			break;
#endif //(_WIN32_IE >= 0x0400)
#if (_WIN32_IE >= 0x0500)
		case PSN_TRANSLATEACCELERATOR:
			{
				LPPSHNOTIFY lpPSHNotify = (LPPSHNOTIFY)lParam;
				lResult = pT->OnTranslateAccelerator((LPMSG)lpPSHNotify->lParam) ? PSNRET_MESSAGEHANDLED : PSNRET_NOERROR;
			}
			break;
		case PSN_QUERYINITIALFOCUS:
			{
				LPPSHNOTIFY lpPSHNotify = (LPPSHNOTIFY)lParam;
				lResult = (LRESULT)pT->OnQueryInitialFocus((HWND)lpPSHNotify->lParam);
			}
			break;
#endif //(_WIN32_IE >= 0x0500)

#endif //!_WTL_NEW_PAGE_NOTIFY_HANDLERS
		default:
			bHandled = FALSE;	// not handled
		}

		return lResult;
	}

// Overridables
	// NOTE: Define _WTL_NEW_PAGE_NOTIFY_HANDLERS to use new notification
	// handlers that return direct values without any restrictions
#ifdef _WTL_NEW_PAGE_NOTIFY_HANDLERS
	int OnSetActive()
	{
		// 0 = allow activate
		// -1 = go back that was active
		// page ID = jump to page
		return 0;
	}
	BOOL OnKillActive()
	{
		// FALSE = allow deactivate
		// TRUE = prevent deactivation
		return FALSE;
	}
	int OnApply()
	{
		// PSNRET_NOERROR = apply OK
		// PSNRET_INVALID = apply not OK, return to this page
		// PSNRET_INVALID_NOCHANGEPAGE = apply not OK, don't change focus
		return PSNRET_NOERROR;
	}
	void OnReset()
	{
	}
	BOOL OnQueryCancel()
	{
		// FALSE = allow cancel
		// TRUE = prevent cancel
		return FALSE;
	}
	int OnWizardBack()
	{
		// 0  = goto previous page
		// -1 = prevent page change
		// >0 = jump to page by dlg ID
		return 0;
	}
	int OnWizardNext()
	{
		// 0  = goto next page
		// -1 = prevent page change
		// >0 = jump to page by dlg ID
		return 0;
	}
	INT_PTR OnWizardFinish()
	{
		// FALSE = allow finish
		// TRUE = prevent finish
		// HWND = prevent finish and set focus to HWND (CommCtrl 5.80 only)
		return FALSE;
	}
	void OnHelp()
	{
	}
#if (_WIN32_IE >= 0x0400)
	BOOL OnGetObject(LPNMOBJECTNOTIFY /*lpObjectNotify*/)
	{
		return FALSE;	// not processed
	}
#endif //(_WIN32_IE >= 0x0400)
#if (_WIN32_IE >= 0x0500)
	int OnTranslateAccelerator(LPMSG /*lpMsg*/)
	{
		// PSNRET_NOERROR - message not handled
		// PSNRET_MESSAGEHANDLED - message handled
		return PSNRET_NOERROR;
	}
	HWND OnQueryInitialFocus(HWND /*hWndFocus*/)
	{
		// NULL = set focus to default control
		// HWND = set focus to HWND
		return NULL;
	}
#endif //(_WIN32_IE >= 0x0500)

#else //!_WTL_NEW_PAGE_NOTIFY_HANDLERS
	BOOL OnSetActive()
	{
		return TRUE;
	}
	BOOL OnKillActive()
	{
		return TRUE;
	}
	BOOL OnApply()
	{
		return TRUE;
	}
	void OnReset()
	{
	}
	BOOL OnQueryCancel()
	{
		return TRUE;    // ok to cancel
	}
	int OnWizardBack()
	{
		// 0  = goto previous page
		// -1 = prevent page change
		// >0 = jump to page by dlg ID
		return 0;
	}
	int OnWizardNext()
	{
		// 0  = goto next page
		// -1 = prevent page change
		// >0 = jump to page by dlg ID
		return 0;
	}
	BOOL OnWizardFinish()
	{
		return TRUE;
	}
	void OnHelp()
	{
	}
#if (_WIN32_IE >= 0x0400)
	BOOL OnGetObject(LPNMOBJECTNOTIFY /*lpObjectNotify*/)
	{
		return FALSE;	// not processed
	}
#endif //(_WIN32_IE >= 0x0400)
#if (_WIN32_IE >= 0x0500)
	BOOL OnTranslateAccelerator(LPMSG /*lpMsg*/)
	{
		return FALSE;	// not translated
	}
	HWND OnQueryInitialFocus(HWND /*hWndFocus*/)
	{
		return NULL;	// default
	}
#endif //(_WIN32_IE >= 0x0500)

#endif //!_WTL_NEW_PAGE_NOTIFY_HANDLERS
};

// for non-customized pages
template <WORD t_wDlgTemplateID>
class CPropertyPage : public CPropertyPageImpl<CPropertyPage<t_wDlgTemplateID> >
{
public:
	enum { IDD = t_wDlgTemplateID };

	CPropertyPage(_U_STRINGorID title = (LPCTSTR)NULL) : CPropertyPageImpl<CPropertyPage>(title)
	{ }

	DECLARE_EMPTY_MSG_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CAxPropertyPageImpl - property page that hosts ActiveX controls

#ifndef _ATL_NO_HOSTING

// Note: You must #include <atlhost.h> to use these classes

template <class T, class TBase = CPropertyPageWindow>
class ATL_NO_VTABLE CAxPropertyPageImpl : public CPropertyPageImpl< T, TBase >
{
public:
// Data members
	HGLOBAL m_hInitData;
	HGLOBAL m_hDlgRes;
	HGLOBAL m_hDlgResSplit;

// Constructor/destructor
	CAxPropertyPageImpl(_U_STRINGorID title = (LPCTSTR)NULL) : 
			CPropertyPageImpl< T, TBase >(title),
			m_hInitData(NULL), m_hDlgRes(NULL), m_hDlgResSplit(NULL)
	{
		T* pT = static_cast<T*>(this);
		pT;	// avoid level 4 warning

		// initialize ActiveX hosting and modify dialog template
		AtlAxWinInit();

		HINSTANCE hInstance = _Module.GetResourceInstance();
		LPCTSTR lpTemplateName = MAKEINTRESOURCE(pT->IDD);
		HRSRC hDlg = ::FindResource(hInstance, lpTemplateName, (LPTSTR)RT_DIALOG);
		if(hDlg != NULL)
		{
			HRSRC hDlgInit = ::FindResource(hInstance, lpTemplateName, (LPTSTR)_ATL_RT_DLGINIT);

			BYTE* pInitData = NULL;
			if(hDlgInit != NULL)
			{
				m_hInitData = ::LoadResource(hInstance, hDlgInit);
				pInitData = (BYTE*)::LockResource(m_hInitData);
			}

			m_hDlgRes = ::LoadResource(hInstance, hDlg);
			DLGTEMPLATE* pDlg = (DLGTEMPLATE*)::LockResource(m_hDlgRes);
			LPCDLGTEMPLATE lpDialogTemplate = _DialogSplitHelper::SplitDialogTemplate(pDlg, pInitData);
			if(lpDialogTemplate != pDlg)
				m_hDlgResSplit = ::GlobalHandle(lpDialogTemplate);

			// set up property page to use in-memory dialog template
			if(lpDialogTemplate != NULL)
			{
				m_psp.dwFlags |= PSP_DLGINDIRECT;
				m_psp.pResource = lpDialogTemplate;
			}
			else
			{
				ATLASSERT(FALSE && _T("CAxPropertyPageImpl - ActiveX initializtion failed!"));
			}
		}
		else
		{
			ATLASSERT(FALSE && _T("CAxPropertyPageImpl - Cannot find dialog template!"));
		}
	}

	~CAxPropertyPageImpl()
	{
		if(m_hInitData != NULL)
		{
			UnlockResource(m_hInitData);
			::FreeResource(m_hInitData);
		}
		if(m_hDlgRes != NULL)
		{
			UnlockResource(m_hDlgRes);
			::FreeResource(m_hDlgRes);
		}
		if(m_hDlgResSplit != NULL)
		{
			::GlobalFree(m_hDlgResSplit);
		}
	}

// Methods
	// call this one to handle keyboard message for ActiveX controls
	BOOL PreTranslateMessage(LPMSG pMsg)
	{
		if ((pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST) &&
		   (pMsg->message < WM_MOUSEFIRST || pMsg->message > WM_MOUSELAST))
			return FALSE;
		// find a direct child of the dialog from the window that has focus
		HWND hWndCtl = ::GetFocus();
		if (IsChild(hWndCtl) && ::GetParent(hWndCtl) != m_hWnd)
		{
			do
			{
				hWndCtl = ::GetParent(hWndCtl);
			}
			while (::GetParent(hWndCtl) != m_hWnd);
		}
		// give controls a chance to translate this message
		return (BOOL)::SendMessage(hWndCtl, WM_FORWARDMSG, 0, (LPARAM)pMsg);
	}

// Overridables
#if (_WIN32_IE >= 0x0500)
	// new default implementation for ActiveX hosting pages
	BOOL OnTranslateAccelerator(LPMSG lpMsg)
	{
		T* pT = static_cast<T*>(this);
		return pT->PreTranslateMessage(lpMsg);
	}
#endif //(_WIN32_IE >= 0x0500)
};

// for non-customized pages
template <WORD t_wDlgTemplateID>
class CAxPropertyPage : public CAxPropertyPageImpl<CAxPropertyPage<t_wDlgTemplateID> >
{
public:
	enum { IDD = t_wDlgTemplateID };

	CAxPropertyPage(_U_STRINGorID title = (LPCTSTR)NULL) : CAxPropertyPageImpl<CAxPropertyPage>(title)
	{ }

#if (_WIN32_IE >= 0x0500)
	// not empty so we handle accelerators
	BEGIN_MSG_MAP(CAxPropertyPage)
		CHAIN_MSG_MAP(CAxPropertyPageImpl<CAxPropertyPage<t_wDlgTemplateID> >)
	END_MSG_MAP()
#else //!(_WIN32_IE >= 0x0500)
	DECLARE_EMPTY_MSG_MAP()
#endif //!(_WIN32_IE >= 0x0500)
};

#endif //_ATL_NO_HOSTING

#endif // __PROPSHEET_H__
