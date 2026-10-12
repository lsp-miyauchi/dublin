// drawlist.h : CDrawListView クラスのインターフェイス
//


#pragma once

// CDrawListView ウィンドウ

#define ID_HEADERCTRL					1001

////////////////////////////////////////////////////////////////////////
//

class CDLItem
{
public:
	UINT m_state;				// state
	void* m_data;				// data
	CString m_tiptext;			// infotip text

	CDLItem() : m_state(0), m_data(NULL)
	{
		m_tiptext.Empty();
	}
	~CDLItem()
	{
	}
	void Init(UINT state, void* data)
	{
		m_state = 0;
		m_data = data;
		m_tiptext.Empty();
	}
	__inline void Clear() { SetTipText(NULL); }
	__inline void SetState(UINT state, UINT mask) { m_state = (m_state & ~mask) | state; }
	__inline void SetTipText(LPCTSTR tip) { m_tiptext = tip; }
	const CDLItem& operator =(const CDLItem& itemSrc)
	{
		m_state = itemSrc.m_state;
		m_data = itemSrc.m_data;
		m_tiptext = itemSrc.m_tiptext;
		return *this;
	}
};

////////////////////////////////////////////////////////////////////////
//

class CDrawListView : public CWindowImpl<CDrawListView>, public CThemeImpl<CDrawListView>
{
public:
	CDrawListView();
	~CDrawListView();

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}


BEGIN_MSG_MAP(CDrawListView)
	CHAIN_MSG_MAP(CThemeImpl<CDrawListView>)
	MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseMessages)
	ON_WM_CREATE()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_CANCELMODE()
	ON_WM_DESTROY()
	ON_WM_GETDLGCODE()
	ON_NOTIFY_CODE(TTN_NEEDTEXT, OnNotifyToolTipText)
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(DLM_GETSEL, OnGetSel)
	ON_MESSAGE(DLM_SETSEL, OnSetSel)
	ON_MESSAGE(DLM_GETCURSEL, OnGetCurSel)
	ON_MESSAGE(DLM_SETCURSEL, OnSetCurSel)
	ON_MESSAGE(DLM_GETITEMSIZE, OnGetItemSize)
	ON_MESSAGE(DLM_SETITEMSIZE, OnSetItemSize)
	ON_MESSAGE(DLM_ADDITEM, OnAddItem)
	ON_MESSAGE(DLM_INSERTITEM, OnInsertItem)
	ON_MESSAGE(DLM_DELETEITEM, OnDeleteItem)
	ON_MESSAGE(DLM_RESETCONTENT, OnResetContent)
	ON_MESSAGE(DLM_SETITEMDATA, OnSetItemData)
	ON_MESSAGE(DLM_GETITEMDATA, OnGetItemData)
	ON_MESSAGE(DLM_GETCOUNT, OnGetCount)
	ON_MESSAGE(DLM_GETSELCOUNT, OnGetSelCount)
	ON_MESSAGE(DLM_GETITEMRECT, OnGetItemRect)
	ON_MESSAGE(DLM_ITEMFROMPOINT, OnItemFromPoint)
	ON_MESSAGE(DLM_GETANCHORPOINT, OnGetAnchorPoint)
	ON_MESSAGE(DLM_GETSELITEMS, OnGetSelItems)
	ON_MESSAGE(DLM_GETTOPINDEX, OnGetTopIndex)
	ON_MESSAGE(DLM_SETTOPINDEX, OnSetTopIndex)
	ON_MESSAGE(DLM_GETANCHORINDEX, OnGetAnchorIndex)
	ON_MESSAGE_VOID(DLM_SETANCHORINDEX, OnSetAnchorIndex)
	ON_MESSAGE(DLM_GETCARETINDEX, OnGetCaretIndex)
	ON_MESSAGE(DLM_SETCARETINDEX, OnSetCaretIndex)
	ON_MESSAGE(DLM_GETDROPTARGETINDEX, OnGetDropTargetIndex)
	ON_MESSAGE_VOID(DLM_SETDROPTARGETINDEX, OnSetDropTargetIndex)
	ON_MESSAGE(DLM_GETNEXTITEM, OnGetNextItem)
	ON_MESSAGE(DLM_ISITEMVISIBLE, OnIsItemVisible)
	ON_MESSAGE(DLM_ENSUREVISIBLE, OnEnsureVisible)
	ON_MESSAGE(DLM_REDRAWITEMS, OnRedrawItems)
	ON_MESSAGE(DLM_REDRAWSELITEMS, OnRedrawSelItems)
	ON_MESSAGE(DLM_GETSPACING, OnGetSpacing)
	ON_MESSAGE(DLM_SETSPACING, OnSetSpacing)
	ON_MESSAGE(DLM_SETOWNER, OnSetOwner)
	ON_MESSAGE_VOID(DLM_SETEXTENDEDSTYLE, OnSetExtendedStyle)
	ON_MESSAGE(DLM_GETEXTENDEDSTYLE, OnGetExtendedStyle)
	ON_MESSAGE_VOID(DLM_SETHORIZONTALEXTENT, OnSetHorizontalExtent)
	ON_MESSAGE(DLM_GETHORIZONTALEXTENT, OnGetHorizontalExtent)
	ON_MESSAGE(DLM_GETBKCOLOR, OnGetBkColor)
	ON_MESSAGE(DLM_SETBKCOLOR, OnSetBkColor)
	ON_MESSAGE(DLM_SETCOUNT, OnSetCount)
	ON_MESSAGE(DLM_GETLISTSTYLE, OnGetListStyle)
	ON_MESSAGE_VOID(DLM_SETLISTSTYLE, OnSetListStyle)
	ON_MESSAGE(DLM_GETHEADER, OnGetHeaderCtrl)
	ON_MESSAGE_VOID(DLM_SETHEADER, OnSetHeaderCtrl)
	ON_MESSAGE_VOID(DLM_LAYOUT, OnLayout)
	ON_MESSAGE(DLM_SETINFOTIP, OnSetInfoTip)
END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnPaint();
	void OnSize(UINT nType, int cx, int cy);
	BOOL OnEraseBkgnd(HDC hdc);
	void OnNcPaint(HRGN hrgn, BOOL& bHandled);
	__inline void OnLButtonDown(UINT nFlags, CPoint point) { MouseButtonDown(nFlags, point); }
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnLButtonDblClk(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);
	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	void OnRButtonDown(UINT nFlags, CPoint point);
	void OnMButtonDown(UINT nFlags, CPoint point);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnSetFocus(HWND hwndOld);
	void OnKillFocus(HWND hwndNew);
	void OnCancelMode();
	void OnDestroy();
	void OnHScroll(UINT nSBCode, UINT nPos, HWND hwndScrollBar);
	void OnVScroll(UINT nSBCode, UINT nPos, HWND hwndScrollBar);
	__inline UINT OnGetDlgCode() { return DLGC_WANTARROWS | DLGC_WANTCHARS; }
	LRESULT OnMouseMessages(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnNotifyToolTipText(int code, NMHDR* phdr, LRESULT* pResult);
	LRESULT OnMouseHover(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	__inline LRESULT OnSetSel(WPARAM wParam, LPARAM lParam) { SetSel((int)lParam, (bool)(wParam != 0)); return 0; }
	__inline LRESULT OnGetCurSel(WPARAM wParam, LPARAM lParam) { return (GetStyle() & DLS_SINGLESEL) ? m_selindex : -1; }
	__inline LRESULT OnSetCurSel(WPARAM wParam, LPARAM lParam) { return (LRESULT)SelectIndex((int)wParam, (bool)(lParam != 0)); }
	__inline LRESULT OnAddItem(WPARAM wParam, LPARAM lParam) { return AddItem((void*)lParam); }
	__inline LRESULT OnInsertItem(WPARAM wParam, LPARAM lParam) { return InsertItem((int)wParam, (void*)lParam); }
	LRESULT OnDeleteItem(WPARAM wParam, LPARAM lParam);
	LRESULT OnResetContent(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetItemData(WPARAM wParam, LPARAM lParam);
	__inline LRESULT OnGetItemData(WPARAM wParam, LPARAM lParam) { return (LRESULT)GetItemData((int)wParam); }
	__inline LRESULT OnGetCount(WPARAM wParam, LPARAM lParam) { return (LRESULT)GetCount(); }
	__inline LRESULT OnGetSelCount(WPARAM wParam, LPARAM lParam) { return (LRESULT)GetSelCount(); }
	__inline LRESULT OnGetItemRect(WPARAM wParam, LPARAM lParam) { return GetItemRect((int)wParam, (RECT*)lParam); }
	__inline LRESULT OnItemFromPoint(WPARAM wParam, LPARAM lParam) { return ItemFromPoint(CPoint(LOWORD(lParam), HIWORD(lParam))); }
	__inline LRESULT OnGetSel(WPARAM wParam, LPARAM lParam) { return GetSel((int)wParam); }
	__inline LRESULT OnGetSelItems(WPARAM wParam, LPARAM lParam) { return GetSelItems((int)wParam, (int*)lParam); }
	__inline LRESULT OnGetTopIndex(WPARAM wParam, LPARAM lParam) { return GetTopIndex(); }
	__inline LRESULT OnSetTopIndex(WPARAM wParam, LPARAM lParam) { return SetTopIndex((int)wParam); }
	__inline LRESULT OnGetAnchorIndex(WPARAM wParam, LPARAM lParam) { return m_ancindex; }
	__inline void OnSetAnchorIndex(WPARAM wParam, LPARAM lParam) { SetAnchorIndex((int)wParam); }
	__inline LRESULT OnGetAnchorPoint(WPARAM wParam, LPARAM lParam) { return (LRESULT)GetAnchorPoint((POINT*)lParam); }
	__inline LRESULT OnGetDropTargetIndex(WPARAM wParam, LPARAM lParam) { return m_dropindex; }
	__inline void OnSetDropTargetIndex(WPARAM wParam, LPARAM lParam) { SetDropTargetIndex((int)wParam); }
	__inline LRESULT OnGetCaretIndex(WPARAM wParam, LPARAM lParam) { return m_selindex; }
	__inline LRESULT OnSetCaretIndex(WPARAM wParam, LPARAM lParam) { return SetCaretIndex((int)wParam, (bool)(lParam != 0)); }
	__inline LRESULT OnGetNextItem(WPARAM wParam, LPARAM lParam) { return GetNextItem((int)wParam, (int)LOWORD(lParam)); }
	__inline LRESULT OnGetItemSize(WPARAM wParam, LPARAM lParam) { CSize size = GetItemSize(); return MAKELRESULT(size.cx, size.cy); }
	__inline LRESULT OnSetItemSize(WPARAM wParam, LPARAM lParam) { return SetItemSize(CSize((DWORD)lParam), (wParam != 0)); }
	__inline LRESULT OnIsItemVisible(WPARAM wParam, LPARAM lParam) { return IsItemVisible((int)wParam, (bool)(lParam != 0)); }
	__inline LRESULT OnEnsureVisible(WPARAM wParam, LPARAM lParam) { return EnsureVisible((int)wParam, (bool)(lParam != 0)); }
	__inline LRESULT OnRedrawItems(WPARAM wParam, LPARAM lParam) { RedrawItemRange((int)wParam, (int)lParam); return 0; }
	__inline LRESULT OnRedrawSelItems(WPARAM wParam, LPARAM lParam) { RedrawSelItems(); return 0; }
	__inline LRESULT OnGetSpacing(WPARAM wParam, LPARAM lParam) { return MAKELRESULT(m_sizeSpacing.cx, m_sizeSpacing.cy); }
	LRESULT OnSetSpacing(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetOwner(WPARAM wParam, LPARAM lParam);
	__inline void OnSetExtendedStyle(WPARAM wParam, LPARAM lParam) { SetExtendedStyle((DWORD)lParam, (DWORD)wParam); }
	__inline LRESULT OnGetExtendedStyle(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_exStyle; }
	__inline void OnSetHorizontalExtent(WPARAM wParam, LPARAM lParam) { SetHorizontalExtent((int)wParam); }
	__inline LRESULT OnGetHorizontalExtent(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_horzExtent; }
	__inline LRESULT OnGetBkColor(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_clrBkgnd; }
	LRESULT OnSetBkColor(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetCount(WPARAM wParam, LPARAM lParam);
	__inline LRESULT OnGetListStyle(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_listStyle; }
	__inline void OnSetListStyle(WPARAM wParam, LPARAM lParam) { SetListStyle((int)wParam); }
	__inline LRESULT OnGetHeaderCtrl(WPARAM wParam, LPARAM lParam) { return (LRESULT)(m_header.IsWindow() ? m_header.m_hWnd : 0); }
	void    OnSetHeaderCtrl(WPARAM wParam, LPARAM lParam);
	void    OnLayout(WPARAM wParam, LPARAM lParam);
	__inline LRESULT OnSetInfoTip(WPARAM wParam, LPARAM lParam) { return (LRESULT)SetInfoTip((NMDLSETINFOTIP*)lParam); }

	BOOL EraseBkgnd(HDC hdc);
	BOOL MouseButtonDown(UINT nFlags, CPoint point);
	void StandardParentNotify(UINT code);

	// スクロールバー
	int GetScrollLimit(int nBar);
	void CalcBars(void);
	void GetScrollBarState(CSize sizeClient, CSize& needSb, CSize& sizeRange, CPoint& ptMove, BOOL bInsideClient);
	void GetScrollBarSizes(CSize& sizeSb);
	bool GetTrueClientSize(CSize& size, CSize& sizeSb);
	void UpdateBars();
	void ScrollToPosition(POINT pt);
	void ScrollToDevicePosition(POINT ptDev);
	bool OnScroll(UINT nScrollCode, UINT nPos, bool bDoScroll = true);
	bool OnScrollBy(CSize sizeScroll, bool bDoScroll = true);

	// リストボックス
	CDLItem* GetItem(int index);
	void* GetItemData(int index);
	int AddItem(void* data);
	int InsertItem(int iInsert, void* data);
	BOOL GetAnchorPoint(LPPOINT ppt);
	void SetSelections(int index);
	int SelectIndex(int index, bool fPartialOK = false);
	void RedrawItemRange(int iStart, int iEnd);
	__inline void RedrawItem(int index) { RedrawItemRange(index, index); }
	void RedrawSelItems();
	void SetAnchorIndex(int index);
	void SetDropTargetIndex(int index);
	__inline int GetCount() { return (int)m_array.GetCount(); }
	__inline bool IsBadIndex(int index) { return (index < 0 || index >= m_array.GetCount()); }
	void GetAbsPos(POINT* ppt);
	int GetItemRectAbs(int index, RECT* prc);
	int GetItemRect(int index, RECT* prc);
	CSize GetItemSize();
	int ItemFromPoint(POINT pt);
	bool GetSel(int index);
	void SetSel(int index, bool bSelect, bool bNotify = true);
	int SetCaretIndex(int index, bool fPartialOK = false);
	int SelItemRange(bool bSelect, int nStartItem, int nEndItem);
	int GetSelCount();
	int GetSelItems(int arraysize, int* array);
	int GetTopIndex();
	int SetTopIndex(int index);
	int GetNextItem(int iStart, int flags);
	bool SetItemSize(CSize sizeItem, bool bRedraw = true);
	bool IsItemVisible(int index, bool fPartialOK);
	bool EnsureVisible(int index, bool fPartialOK);
	bool EnableToolTips(bool bEnable);
	void SetExtendedStyle(DWORD newStyle, DWORD mask);
	void SetListStyle(int newStyle);
	void SetHorizontalExtent(int horzExtent);
	bool StartDragDrop();
	bool SetInfoTip(NMDLSETINFOTIP* pInfoTipStruct);

	__inline bool IsSingleSel() { return ((GetStyle() & DLS_SINGLESEL) != 0); }

	__inline bool IsListStyle() { return (m_listStyle == DLS_STYLE_LIST); }
	__inline bool IsReportStyle() { return (m_listStyle == DLS_STYLE_REPORT); }
	__inline bool IsIconStyle() { return (m_listStyle == DLS_STYLE_ICON); }

	__inline bool IsExOneClickActivate() { return ((m_exStyle & DLS_EX_ONECLICKACTIVATE) != 0); }
	__inline bool IsExContextMenu() { return ((m_exStyle & DLS_EX_CONTEXTMENU) != 0); }
	__inline bool IsExToolTip() { return ((m_exStyle & DLS_EX_TOOLTIP) != 0); }
	__inline bool IsExHotTrack() { return ((m_exStyle & DLS_EX_HOTTRACK) != 0); }
	__inline bool IsExHover() { return ((m_exStyle & (DLS_EX_HOTTRACK | DLS_EX_ONECLICKACTIVATE)) != 0); }
	__inline bool IsExWrapCursor() { return ((m_exStyle & DLS_EX_WRAPCURSOR) != 0); }

	// 生成された、メッセージ割り当て関数
public:
	CWindow m_owner;

	int m_col;					// クライアント領域列数
	int m_row;					// クライアント領域あたりの行数
	int m_selindex;				// 現在フォーカス中のインデクス
	int m_ancindex;				// アンカーインデクス
	bool m_bInsideUpdate;		//
	CSize m_sizeItem;			// 1アイテムの縦横サイズ
	CSize m_sizePage;
	CSize m_sizeTotal;
	CSize m_sizeSpacing;
	CSize m_sizeClient;			// クライアントサイズ
	int m_horzExtent;
	CAtlArray<CDLItem> m_array;
	HFONT m_hfont;
	COLORREF m_clrBkgnd;		// 背景色

	CToolTipCtrl m_tip;
	int m_hit;
	bool m_bEnableTip;
	DWORD m_exStyle;
	DWORD m_listStyle;

	// Capture the mouse for zooming in
	bool m_bCaptured;
	CPoint m_ptSel;
	CRect m_rcSel;
	int m_dragancindex;
	int m_dragbefindex;
	int m_dropindex;
	BOOL m_bTrackMouseEvent;
	int m_hoverindex;
	int m_prevhoverindex;

	CHeaderCtrl m_header;
	int m_numWheelScrollLines;
};

