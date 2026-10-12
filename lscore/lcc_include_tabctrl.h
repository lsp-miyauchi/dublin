///////////////////////////////////////////////////////////////////////////////
// CLccTabExCtrl

#pragma once
#ifndef __lcc_include_tabctrl_h__
#define __lcc_include_tabctrl_h__


template <class TBase>
class CLccTabExCtrlT : public TBase
{
public:
// Constructors
	CLccTabExCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CLccTabExCtrlT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return WC_LSC_TAB_EX;
	}

	HIMAGELIST GetImageList() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HIMAGELIST)::SendMessage(m_hWnd, TCM_GETIMAGELIST, 0, 0L);
	}

	HIMAGELIST SetImageList(HIMAGELIST hImageList)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HIMAGELIST)::SendMessage(m_hWnd, TCM_SETIMAGELIST, 0, (LPARAM)hImageList);
	}

	int GetItemCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_GETITEMCOUNT, 0, 0L);
	}

	BOOL GetItem(int nItem, LPTCITEM pTabCtrlItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TCM_GETITEM, nItem, (LPARAM)pTabCtrlItem);
	}

	DWORD_PTR GetItemData(int nItem) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TCITEM item = { 0 };
		item.mask = TCIF_PARAM;
		if (::SendMessage(m_hWnd, TCM_GETITEM, nItem, (LPARAM)&item))
			return (DWORD_PTR)item.lParam;
		return NULL;
	}

	BOOL SetItemData(int nIndex, DWORD_PTR dwItemData)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TCITEM item = { 0 };
		item.mask = TCIF_PARAM;
		item.lParam = (LPARAM)dwItemData;
		return (BOOL)::SendMessage(m_hWnd, TCM_SETITEM, nIndex, (LPARAM)&item);
	}

	BOOL SetItem(int nItem, LPTCITEM pTabCtrlItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TCM_SETITEM, nItem, (LPARAM)pTabCtrlItem);
	}

	int SetItem(int nItem, UINT mask, LPCTSTR lpszItem, DWORD dwState, DWORD dwStateMask, int iImage, DWORD lParam)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TCITEM item = { 0 };
		item.mask = mask;
		item.pszText = (LPTSTR)lpszItem;
		item.dwState = dwState;
		item.dwStateMask = dwStateMask;
		item.iImage = iImage;
		item.lParam = lParam;
		return (int)::SendMessage(m_hWnd, TCM_SETITEM, nItem, (LPARAM)&item);
	}

	BOOL GetItemRect(int nItem, LPRECT lpRect) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TCM_GETITEMRECT, nItem, (LPARAM)lpRect);
	}

	int GetCurSel() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_GETCURSEL, 0, 0L);
	}

	int SetCurSel(int nItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_SETCURSEL, nItem, 0L);
	}

	SIZE SetItemSize(SIZE size)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		DWORD dwSize = (DWORD)::SendMessage(m_hWnd, TCM_SETITEMSIZE, 0, MAKELPARAM(size.cx, size.cy));
		SIZE sizeRet = { GET_X_LPARAM(dwSize), GET_Y_LPARAM(dwSize) };
		return sizeRet;
	}

	void SetItemSize(int cx, int cy)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_SETITEMSIZE, 0, MAKELPARAM(cx, cy));
	}

	void SetPadding(SIZE size)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_SETPADDING, 0, MAKELPARAM(size.cx, size.cy));
	}

	int GetRowCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_GETROWCOUNT, 0, 0L);
	}

	CToolTipCtrl GetTooltips() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return CToolTipCtrl((HWND)::SendMessage(m_hWnd, TCM_GETTOOLTIPS, 0, 0L));
	}

	void SetTooltips(HWND hWndToolTip)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_SETTOOLTIPS, (WPARAM)hWndToolTip, 0L);
	}

	int GetCurFocus() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_GETCURFOCUS, 0, 0L);
	}

	void SetCurFocus(int nItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_SETCURFOCUS, nItem, 0L);
	}

	int SetMinTabWidth(int nWidth = -1)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_SETMINTABWIDTH, 0, nWidth);
	}

	DWORD GetExtendedStyle() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, TCM_GETEXTENDEDSTYLE, 0, 0L);
	}

	DWORD SetExtendedStyle(DWORD dwExMask, DWORD dwExStyle)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, TCM_SETEXTENDEDSTYLE, dwExMask, dwExStyle);
	}

// Operations
	int InsertItem(int nItem, LPTCITEM pTabCtrlItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_INSERTITEM, nItem, (LPARAM)pTabCtrlItem);
	}

	int InsertItem(int nItem, UINT mask, LPCTSTR lpszItem, int iImage, LPARAM lParam)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TCITEM tci = { 0 };
		tci.mask = mask;
		tci.pszText = (LPTSTR) lpszItem;
		tci.iImage = iImage;
		tci.lParam = lParam;
		return (int)::SendMessage(m_hWnd, TCM_INSERTITEM, nItem, (LPARAM)&tci);
	}

	int InsertItem(int nItem, LPCTSTR lpszItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TCITEM tci = { 0 };
		tci.mask = TCIF_TEXT;
		tci.pszText = (LPTSTR) lpszItem;
		return (int)::SendMessage(m_hWnd, TCM_INSERTITEM, nItem, (LPARAM)&tci);
	}

	BOOL DeleteItem(int nItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TCM_DELETEITEM, nItem, 0L);
	}

	BOOL DeleteAllItems()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TCM_DELETEALLITEMS, 0, 0L);
	}

	void AdjustRect(BOOL bLarger, LPRECT lpRect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_ADJUSTRECT, bLarger, (LPARAM)lpRect);
	}

	void RemoveImage(int nImage)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_REMOVEIMAGE, nImage, 0L);
	}

	int HitTest(TC_HITTESTINFO* pHitTestInfo) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_HITTEST, 0, (LPARAM)pHitTestInfo);
	}

	void DeselectAll(BOOL bExcludeFocus = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_DESELECTALL, bExcludeFocus, 0L);
	}

	BOOL HighlightItem(int nIndex, BOOL bHighlight = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TCM_HIGHLIGHTITEM, nIndex, MAKELPARAM(bHighlight, 0));
	}

	DWORD GetItemState(int nIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));

		TCITEM item = { 0 };
		item.mask = TCIF_STATE;
		if (::SendMessage(m_hWnd, TCM_GETITEM, nIndex, (LPARAM)&item))
			return item.dwState;
		return (DWORD)-1;
	}

	BOOL MarkItem(int nIndex, BOOL bMark = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));

		TCITEM item = { 0 };
		item.mask = TCIF_STATE;
		item.dwState = bMark ? TCIS_MARKED : 0;
		item.dwStateMask = TCIS_MARKED;
		return (BOOL)::SendMessage(m_hWnd, TCM_SETITEM, nIndex, (LPARAM)&item);
	}

	BOOL LockItem(int nIndex, BOOL bLock = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));

		TCITEM item = { 0 };
		item.mask = TCIF_STATE;
		item.dwState = bLock ? TCIS_LOCKED : 0;
		item.dwStateMask = TCIS_LOCKED;
		return (BOOL)::SendMessage(m_hWnd, TCM_SETITEM, nIndex, (LPARAM)&item);
	}

	BOOL TabMoveTo(int indexTarget, int indexSource)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TCM_TABMOVETO, (WPARAM)indexTarget, (LPARAM)indexSource);
	}

	BOOL TabExchange(int index1, int index2)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, TCM_TABEXCHANGE, (WPARAM)index1, (LPARAM)index2);
	}

	void SetNotifyWnd(HWND hWnd)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_SETPARENT, (WPARAM)hWnd, 0L);
	}

	COLORREF GetPaneColor()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, TCM_GETPANECOLOR, 0, 0);
	}

	COLORREF SetPaneColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, TCM_SETPANECOLOR, 0, (LPARAM)clr);
	}

	HBITMAP CreateDragImage(int index, POINT* ppt = NULL)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HBITMAP)::SendMessage(m_hWnd, TCM_CREATEDRAGIMAGE, (WPARAM)index, (LPARAM)ppt);
	}

	void GetBarMetrics(NMTCMETRICS* ptcm)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_GETBARMETRICS, 0, (LPARAM)ptcm);
	}

	void SetBarMetrics(NMTCMETRICS* ptcm)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_SETBARMETRICS, 0, (LPARAM)ptcm);
	}

	HBITMAP GetBackgroundImage()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HBITMAP)::SendMessage(m_hWnd, TCM_GETBKGNDIMAGE, 0, 0);
	}

	HBITMAP SetBackgroundImage(HBITMAP hbm)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HBITMAP)::SendMessage(m_hWnd, TCM_SETBKGNDIMAGE, 0, (LPARAM)hbm);
	}

};

typedef CLccTabExCtrlT<CWindow>   CLccTabExCtrl;



#endif		// __lcc_include_tabctrl_h__
