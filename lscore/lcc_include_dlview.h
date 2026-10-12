/////////////////////////////////////////////////////////////////////////////
// CDrawListCtrl

#pragma once
#ifndef __lcc_include_dlview_h__
#define __lcc_include_dlview_h__

template <class TBase>
class CDrawListCtrlT : public TBase
{
public:
// Constructors
	CDrawListCtrlT(HWND hWnd = NULL) : TBase(hWnd) { }

	CDrawListCtrlT< TBase >& operator=(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, _U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return CWindow::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return WC_LSC_LISTVIEW;
	}

	int GetCount()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_GETCOUNT, 0L, 0L);
	}
	BOOL ResetContent()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, DLM_RESETCONTENT, 0L, 0L);
	}
	int AddItem(void* data)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_ADDITEM, 0L, (LPARAM)data);
	}
	int InsertItem(int index, void* data)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_INSERTITEM, (WPARAM)index, (LPARAM)data);
	}
	int DeleteItem(int index)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_DELETEITEM, (WPARAM)index, 0L);
	}
	void* GetItemData(int index)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (void*)(ULONG_PTR)::SendMessage(m_hWnd, DLM_GETITEMDATA, (WPARAM)index, 0L);
	}
	int SetItemData(int index, void* data)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_SETITEMDATA, (WPARAM)index, (LPARAM)data);
	}
	int GetCurSel()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_GETCURSEL, 0L, 0L);
	}
	int SetCurSel(int index, bool fPartialOK = false)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_SETCURSEL, (WPARAM)index, (LPARAM)fPartialOK);
	}
	int SetSel(int index, BOOL fSelect = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_SETSEL, (WPARAM)fSelect, (LPARAM)index);
	}
	int SelItemRange(BOOL fSelect, int first, int last)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_SELITEMRANGE, (WPARAM)fSelect, MAKELPARAM(first, last));
	}
	BOOL GetSel(int index)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, DLM_GETSEL, (WPARAM)index, 0L);
	}
	int GetSelCount()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_GETSELCOUNT, 0L, 0L);
	}
	int GetSelItems(int cItems, int* lpItems)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_GETSELITEMS, (WPARAM)cItems, (LPARAM)lpItems);
	}
	int SetMark(int index, BOOL fMark = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_SETMARK, (WPARAM)fMark, (LPARAM)index);
	}
	int MarkItemRange(BOOL fMark, int first, int last)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_MARKITEMRANGE, (WPARAM)fMark, MAKELPARAM(first, last));
	}
	BOOL GetMark(int index)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, DLM_GETMARK, (WPARAM)index, 0L);
	}
	int GetMarkCount()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_GETMARKCOUNT, 0L, 0L);
	}
	int GetMarkItems(int cItems, int* lpItems)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_GETMARKITEMS, (WPARAM)cItems, (LPARAM)lpItems);
	}
	int GetTopIndex()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_GETTOPINDEX, 0L, 0L);
	}
	int SetTopIndex(int indexTop)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_SETTOPINDEX, (WPARAM)indexTop, 0L);
	}
	int GetItemRect(int index, LPRECT lprc)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_GETITEMRECT, (WPARAM)index, (LPARAM)lprc);
	}
	int SetCaretIndex(int index, BOOL fPartialOK = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_SETCARETINDEX, (WPARAM)index, (LPARAM)fPartialOK);
	}
	int GetCaretIndex()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_GETCARETINDEX, 0L, 0L);
	}
	int SetAnchorIndex(int index)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_SETANCHORINDEX, (WPARAM)index, 0L);
	}
	int GetAnchorIndex()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_GETANCHORINDEX, 0L, 0L);
	}
	void SetDropTargetIndex(int index)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, DLM_SETDROPTARGETINDEX, (WPARAM)index, 0L);
	}
	int GetDropTargetIndex()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_GETDROPTARGETINDEX, 0L, 0L);
	}
	int ItemFromPoint(POINT pt)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
	}
	BOOL GetAnchorPoint(POINT* ppt)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, DLM_GETANCHORPOINT, 0L, (LPARAM)ppt);
	}
	DWORD GetItemSize()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, DLM_GETITEMSIZE, 0L, 0L);
	}
	int SetItemSize(int cx, int cy)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_SETITEMSIZE, 0L, MAKELPARAM(cx, cy));
	}
	int SetItemSize(SIZE size)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_SETITEMSIZE, 0L, MAKELPARAM(size.cx, size.cy));
	}
	int GetNextItem(int index, UINT flags)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_GETNEXTITEM, (WPARAM)index, MAKELPARAM(flags, 0));
	}
	BOOL IsItemVisible(int index, BOOL fPartialOK)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, DLM_ISITEMVISIBLE, (WPARAM)index, (LPARAM)fPartialOK);
	}
	BOOL EnsureVisible(int index, BOOL fPartialOK)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, DLM_ENSUREVISIBLE, (WPARAM)index, (LPARAM)fPartialOK);
	}
	int DeleteAllItems()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_DELETEALLITEMS, 0L, 0L);
	}
	int RedrawItems(int iStart, int iEnd)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_REDRAWITEMS, (WPARAM)iStart, (LPARAM)iEnd);
	}
	int RedrawSelItems()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_REDRAWSELITEMS, 0, 0);
	}
	DWORD GetSpacing()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, DLM_GETSPACING, 0L, 0L);
	}
	int SetSpacing(SIZE size)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_SETSPACING, 0L, MAKELPARAM(size.cx, size.cy));
	}
	HWND SetOwner(HWND hwndOwner)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HWND)::SendMessage(m_hWnd, DLM_SETOWNER, 0, (LPARAM)hwndOwner);
	}
	void SetExtendedStyle(DWORD dwExStyle, DWORD dwExMask = 0)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, DLM_SETEXTENDEDSTYLE, dwExMask, dwExStyle);
	}
	DWORD GetExtendedStyle() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, DLM_GETEXTENDEDSTYLE, 0, 0L);
	}
	void EnableToolTips(BOOL bEnable = TRUE)
	{
		SetExtendedStyle(bEnable ? DLS_EX_TOOLTIP : 0, DLS_EX_TOOLTIP);
	}
	COLORREF GetBkColor() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, DLM_GETBKCOLOR, 0, 0L);
	}
	COLORREF SetBkColor(COLORREF clr)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, DLM_SETBKCOLOR, 0, (LPARAM)clr);
	}
	int SetCount(int cnt)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_SETCOUNT, (WPARAM)cnt, 0);
	}
	void SetHorizontalExtent(int extent)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, DLM_SETHORIZONTALEXTENT, (WPARAM)extent, 0);
	}
	int GetHorizontalExtent()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_GETHORIZONTALEXTENT, 0, 0);
	}
	int GetListStyle()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, DLM_GETLISTSTYLE, 0, 0);
	}
	void SetListStyle(int style)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, DLM_SETLISTSTYLE, (WPARAM)style, 0);
	}
	HWND GetHeaderCtrl()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HWND)::SendMessage(m_hWnd, DLM_GETHEADER, 0, 0);
	}
	void SetHeaderCtrl(HWND hHeader)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, DLM_SETHEADER, 0, (LPARAM)hHeader);
	}
	void Layout(RECT* prc)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, DLM_LAYOUT, 0, (LPARAM)prc);
	}
	void CreateHeaderCtrlEx(HWND hwndParent, UINT id, DWORD style = WS_CHILD | HDS_HORZ | HDS_BUTTONS)
	{
		CHeaderCtrl hdr;
		style |= WS_CHILD;
		hdr.Create(GetParent(), NULL, NULL, style, 0, id, NULL);
		SetHeaderCtrl(hdr.Detach());
/* --> LCCHeader を使う場合 -------------------------------------
		style |= WS_CHILD;
		HWND hwnd = ::CreateWindowEx(0, WC_LSC_HEADER, NULL, style, 0, 0, 2, 2, hwndParent, (HMENU)id, _AtlBaseModule.GetModuleInstance(), NULL);
		SetHeaderCtrl(hwnd);
<------------------------------------------------------------- */
	}
	void CreateHeaderCtrl(UINT id)
	{
		CreateHeaderCtrlEx(GetParent(), id);
	}
	void ResetHeaderWindowPosition()
	{
		ATLASSERT(::IsWindow(m_hWnd));
//@		if (GetListStyle() == DLS_STYLE_REPORT) {
			CHeaderCtrl hdr = GetHeaderCtrl();
			if (hdr.IsWindow()) {
				hdr.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE);
				hdr.RedrawWindow(NULL, NULL, RDW_UPDATENOW);
			}
//@		}
	}
	BOOL SetHeaderColumnWidth(int nCol, int cx)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		CHeaderCtrl hdr = GetHeaderCtrl();
		if (hdr.IsWindow()) {
			HDITEM hd;
			hd.mask = HDI_WIDTH;
			hd.cxy = cx;
			return (BOOL)hdr.SetItem(nCol, &hd);
		}
		return FALSE;
	}
	int InsertHeaderColumn(int nCol, HDITEM* pColumn)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		CHeaderCtrl hdr = GetHeaderCtrl();
		if (hdr.IsWindow())
			return (int)hdr.InsertItem(nCol, pColumn);
		return -1;
	}
	BOOL DeleteHeaderColumn(int nCol)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		CHeaderCtrl hdr = GetHeaderCtrl();
		if (hdr.IsWindow())
			return (BOOL)hdr.DeleteItem(nCol);
		return FALSE;
	}
	int GetHeaderColumnCount()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		CHeaderCtrl hdr = GetHeaderCtrl();
		if (hdr.IsWindow())
			return (int)hdr.GetItemCount();
		return -1;
	}
	BOOL GetHeaderColumn(int nPos, HDITEM* pHeaderItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		CHeaderCtrl hdr = GetHeaderCtrl();
		if (hdr.IsWindow())
			return (BOOL)hdr.GetItem(nPos, pHeaderItem);
		return FALSE;
	}
	BOOL SetHeaderColumn(int nPos, HDITEM* pHeaderItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		CHeaderCtrl hdr = GetHeaderCtrl();
		if (hdr.IsWindow())
			return (BOOL)hdr.SetItem(nPos, pHeaderItem);
		return FALSE;
	}

	BOOL SetInfoTip(NMDLSETINFOTIP* pInfoTipStruct)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, DLM_SETINFOTIP, (WPARAM)0, (LPARAM)(pInfoTipStruct));
	}
};

typedef CDrawListCtrlT<CWindow>	CDrawListCtrl;


#endif		// __lcc_include_dlview_h__
