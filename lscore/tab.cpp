/*
 * Tab control
 *
 * TODO:
 *
 *  Styles:
 *   TCS_MULTISELECT, TCS_RIGHT, TCS_RIGHTJUSTIFY, TCS_SCROLLOPPOSITE, TCS_SINGLELINE
 *
 *  Extended Styles:
 *   TCS_EX_FLATSEPARATORS, TCS_EX_REGISTERDROP
 *
 *  States:
 *   TCIS_BUTTONPRESSED
 *
 *  Notifications:
 *   NM_RELEASEDCAPTURE, TCN_FOCUSCHANGE, TCN_GETOBJECT
 *   TCN_KEYDOWN
 *
 *  Messages:
 *   TCM_REMOVEIMAGE, TCM_DESELECTALL, TCM_GETEXTENDEDSTYLE, TCM_SETEXTENDEDSTYLE
 *
 *  Macros:
 *   TabCtrl_AdjustRect
 *
 */

#include "stdafx.h"
#include <math.h>
#include "lscore.h"
#include "resource.h"
#include "fnc.h"
#include "common.h"
#include "tooltip_sup.h"
#include "gdiplus_sup.h"

#define FLOOR_INT(n)					((int)floor((n)))

////////////////////////////////////////////////////////////////////////
// Positioning constants

#define DEFAULT_MIN_TAB_WIDTH			16
#define EXTRA_ICON_PADDING				2					// アイコン－テキスト間のパディング
#define MAX_URL_LENGTH					(2048 + 32 + 8)

#define TCIS_MARKED						0x8000

////////////////////////////////////////////////////////////////////////
//

class CTabItem
{
public:
	DWORD m_state;
	CString m_strText;
	int m_iImage;
	LPARAM m_param;
	CRect m_rect;

public:
	CTabItem() : m_state(0), m_iImage(-1), m_param(0) { m_strText.Empty(); }
	~CTabItem() { }

};


////////////////////////////////////////////////////////////////////////
//

class CLccTabCtrl :	public CWindowImpl<CLccTabCtrl>,
					public CToolTipUsing<CLccTabCtrl>
{
private:
	// gdi+
	gdiplus_using m_gdiplus_using;

	CWindow m_owner;				// notification window (parent)
	int m_numItem;					// number of tab items
	int m_numRows;					// number of tab rows
	int m_tabHeight;				// height of the tab row
	int m_tabMinWidth;				// タブアイテムに設定する最小幅。この値を -1 にすると、既定のタブ幅が使わる
	CSize m_sizePadding;			// 各タブのアイコンとラベル周囲の水平垂直方向スペース量
	CFont m_font;					// テキストフォント
	CFont m_fontBold;				// 太字テキストフォント
	CFont m_fontMarlett;
	HIMAGELIST m_him;				// イメージリスト
	int m_selIndex;					// the currently selected item
	int m_hotIndex;					// the highlighted item under the mouse
	int m_focusIndex;				// item which has the focus
	UINT m_hotIndexHitTest;
	CAtlArray<CTabItem*> m_array;	// pointer to an array of CTabItem's
	int m_cxMargin;					// クライアント領域水平方向のマージン
	int m_cyTopMargin;				// クライアント領域上側のマージン
	int m_cyBottomMargin;			// クライアント領域下側のマージン
	int m_cxTabSpacing;				// タブ間スペーシング（マイナス値が重なりサイズを示す）
	CSize m_sizeIcon;
	DWORD m_exStyle;				// 拡張スタイル
	bool m_bDoNotRedraw:1;			// flag for redrawing when tab contents is changed
	bool m_bHeightSet:1;			// was the height of the tabs explicitly set?
	bool m_bTrackMouseEvent:1;

	CBrush m_brPanel;

public:
	CLccTabCtrl()
	{
		m_numItem = 0;
		m_numRows = 0;
		m_tabHeight = 0;
		m_tabMinWidth = -1;
		m_sizePadding = CSize(4, 3);
		m_him = NULL;
		m_selIndex = -1;
		m_hotIndex = -1;
		m_focusIndex = -1;
		m_hotIndexHitTest = TCHT_NOWHERE;
		m_cxMargin = 2;
		m_cyTopMargin = 3;
		m_cyBottomMargin = 3;
//@		m_cxTabSpacing = -1;
		m_cxTabSpacing = 0;
		m_sizeIcon = CSize(0, 0);
		m_exStyle = 0;
		m_bDoNotRedraw = false;
		m_bHeightSet = false;
		m_bTrackMouseEvent = false;
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

BEGIN_MSG_MAP(CLccTabCtrl)
	CHAIN_MSG_MAP(CToolTipUsing<CLccTabCtrl>)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_NCCALCSIZE()
	ON_WM_GETDLGCODE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE_VOID(WM_SETREDRAW, OnSetRedraw)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_MESSAGE_VOID(WM_SETFONT, OnSetFont)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_NOTIFY_CODE(TTN_NEEDTEXT, OnNotifyToolTipText)
	ON_MESSAGE_VOID(WM_SETFOCUS, OnSetFocus)
	ON_MESSAGE_VOID(WM_KILLFOCUS, OnKillFocus)
	ON_WM_KEYDOWN()
	ON_WM_NCHITTEST()
	ON_MESSAGE_VOID(WM_STYLECHANGED, OnStyleChanged)
	ON_WM_PRINTCLIENT()
	ON_WM_PAINT()
	ON_MESSAGE(TCM_GETIMAGELIST, OnTcmGetImageList)
	ON_MESSAGE(TCM_SETIMAGELIST, OnTcmSetImageList)
	ON_MESSAGE(TCM_GETITEMCOUNT, OnTcmGetItemCount)
	ON_MESSAGE(TCM_GETITEM, OnTcmGetItem)
	ON_MESSAGE(TCM_SETITEM, OnTcmSetItem)
	ON_MESSAGE(TCM_DELETEITEM, OnTcmDeleteItem)
	ON_MESSAGE(TCM_DELETEALLITEMS, OnTcmDeleteAllItems)
	ON_MESSAGE(TCM_GETITEMRECT, OnTcmGetItemRect)
	ON_MESSAGE(TCM_GETCURSEL, OnTcmGetCurSel)
	ON_MESSAGE(TCM_SETCURSEL, OnTcmSetCurSel)
	ON_MESSAGE(TCM_HITTEST, OnTcmHitTest)
	ON_MESSAGE(TCM_INSERTITEM, OnTcmInsertItem)
	ON_MESSAGE_VOID(TCM_ADJUSTRECT, OnTcmAdjustRect)
	ON_MESSAGE(TCM_SETITEMSIZE, OnTcmSetItemSize)
	ON_MESSAGE(TCM_REMOVEIMAGE, OnTcmRemoveImage)
	ON_MESSAGE_VOID(TCM_SETPADDING, OnTcmSetPadding)
	ON_MESSAGE(TCM_GETROWCOUNT, OnTcmGetRowCount)
	ON_MESSAGE(TCM_GETUNICODEFORMAT, OnTcmGetUnicodeFormat)
	ON_MESSAGE(TCM_SETUNICODEFORMAT, OnTcmSetUnicodeFormat)
	ON_MESSAGE(TCM_HIGHLIGHTITEM, OnTcmHighlightItem)
	ON_MESSAGE(TCM_GETTOOLTIPS, OnTcmGetTooltips)
	ON_MESSAGE_VOID(TCM_SETTOOLTIPS, OnTcmSetTooltips)
	ON_MESSAGE(TCM_GETCURFOCUS, OnTcmGetCurFocus)
	ON_MESSAGE_VOID(TCM_SETCURFOCUS, OnTcmSetCurFocus)
	ON_MESSAGE(TCM_SETMINTABWIDTH, OnTcmSetMinTabWidth)
	ON_MESSAGE(TCM_DESELECTALL, OnTcmDeselectAll)
	ON_MESSAGE(TCM_GETEXTENDEDSTYLE, OnTcmGetExtendedStyle)
	ON_MESSAGE(TCM_SETEXTENDEDSTYLE, OnTcmSetExtendedStyle)
	ON_MESSAGE_VOID(TCM_SETPARENT, OnTcmSetParent)
END_MSG_MAP()

private:
	int OnCreate(CREATESTRUCT* pcs);
	void OnDestroy();
	UINT OnNcCalcSize(BOOL bCalcValidateRect, NCCALCSIZE_PARAMS* pncp);
	__inline UINT OnGetDlgCode() { return DLGC_WANTARROWS | DLGC_WANTCHARS; }
	void OnSize(UINT type, int cx, int cy);
	__inline BOOL OnEraseBkgnd(HDC hdc) { EraseBkgnd(hdc); return TRUE; }
	__inline void OnSetRedraw(WPARAM wParam, LPARAM lParam) { SetRedraw((BOOL)wParam); }
	__inline LRESULT OnGetFont(WPARAM, LPARAM) { return (LRESULT)GetFont(); }
	void OnSetFont(WPARAM wParam, LPARAM);
	void OnLButtonDown(UINT nFlags, CPoint pt);
	void OnLButtonUp(UINT nFlags, CPoint pt);
	void OnLButtonDblClk(UINT nFlags, CPoint pt);
	void OnRButtonDown(UINT nFlags, CPoint pt);
	void OnMouseMove(UINT nFlags, CPoint pt);
	void OnMouseLeave();
	void OnNotifyToolTipText(int id, NMHDR* phdr, LRESULT* pResult);
	__inline void OnSetFocus(WPARAM, LPARAM) { FocusChanging(); }
	__inline void OnKillFocus(WPARAM, LPARAM) { FocusChanging(); }
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	UINT OnNcHitTest(int xPos, int yPos);
	void OnStyleChanged(WPARAM, LPARAM);
	void OnPrintClient(HDC hdc, LPARAM flags);
	void OnPaint();
	__inline LRESULT OnTcmGetImageList(WPARAM, LPARAM) { return (LRESULT)m_him; }
	__inline LRESULT OnTcmSetImageList(WPARAM, LPARAM lParam) { return (LRESULT)SetImageList((HIMAGELIST)lParam); }
	__inline LRESULT OnTcmGetItemCount(WPARAM, LPARAM) { return (LRESULT)m_numItem; }
	__inline LRESULT OnTcmGetItem(WPARAM wParam, LPARAM lParam) { return GetItemT((int)wParam, (TCITEM*)lParam); }
	__inline LRESULT OnTcmSetItem(WPARAM wParam, LPARAM lParam) { return SetItemT((int)wParam, (TCITEM*)lParam); }
	__inline LRESULT OnTcmDeleteItem(WPARAM wParam, LPARAM) { return DeleteItem((int)wParam); }
	__inline LRESULT OnTcmDeleteAllItems(WPARAM, LPARAM) { return DeleteAllItems(); }
	__inline LRESULT OnTcmGetItemRect(WPARAM wParam, LPARAM lParam) { return GetItemRect((int)wParam, (RECT*)lParam); }
	__inline LRESULT OnTcmGetCurSel(WPARAM, LPARAM) { return (LRESULT)m_selIndex; }
	__inline LRESULT OnTcmSetCurSel(WPARAM wParam, LPARAM) { return (LRESULT)SetCurSel((int)wParam); }
	__inline LRESULT OnTcmHitTest(WPARAM, LPARAM lParam) { return HitTest((LPTCHITTESTINFO)lParam); }
	__inline LRESULT OnTcmInsertItem(WPARAM wParam, LPARAM lParam) { return (LRESULT)InsertItem((int)wParam, (TCITEM*)lParam); }
	__inline void OnTcmAdjustRect(WPARAM wParam, LPARAM lParam) { AdjustRect((BOOL)wParam, (LPRECT)lParam); }
	__inline LRESULT OnTcmSetItemSize(WPARAM wParam, LPARAM lParam) { return SetItemSize(CSize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))); }
	__inline LRESULT OnTcmRemoveImage(WPARAM wParam, LPARAM lParam) { return 0; }
	__inline void OnTcmSetPadding(WPARAM, LPARAM lParam) { SetPadding(CSize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))); }
	__inline LRESULT OnTcmGetRowCount(WPARAM, LPARAM) { return (LRESULT)m_numRows; }
	__inline LRESULT OnTcmGetUnicodeFormat(WPARAM, LPARAM) { return TRUE; }
	__inline LRESULT OnTcmSetUnicodeFormat(WPARAM, LPARAM) { return TRUE; }
	__inline LRESULT OnTcmHighlightItem(WPARAM wParam, LPARAM lParam) { return HighlightItem((int)wParam, (BOOL)LOWORD(lParam)); }
	__inline LRESULT OnTcmGetTooltips(WPARAM, LPARAM) { return (LRESULT)GetToolTips(); }
	__inline void OnTcmSetTooltips(WPARAM wParam, LPARAM) { SetToolTips((HWND)wParam); }
	__inline LRESULT OnTcmGetCurFocus(WPARAM, LPARAM) { return GetCurFocus(); }
	__inline void OnTcmSetCurFocus(WPARAM wParam, LPARAM) { SetCurFocus((int)wParam); }
	__inline LRESULT OnTcmSetMinTabWidth(WPARAM, LPARAM lParam) { return SetMinTabWidth((int)lParam); }
	__inline LRESULT OnTcmDeselectAll(WPARAM, LPARAM) { return 0; }
	__inline LRESULT OnTcmGetExtendedStyle(WPARAM, LPARAM) { return (LRESULT)m_exStyle; }
	__inline LRESULT OnTcmSetExtendedStyle(WPARAM wParam, LPARAM lParam) { return (LRESULT)SetExtendedStyle((DWORD)wParam, (DWORD)lParam); }
	__inline void OnTcmSetParent(WPARAM wParam, LPARAM) { ATLASSERT(::IsWindow((HWND)wParam)); m_owner = (HWND)wParam; }

	void OnDraw(CDCHandle dc);
	void DrawInsertMark(CDCHandle dc, int index, BOOL bOn);
	void DrawItem(CDCHandle dc, int index);
	void EraseBkgnd(CDCHandle dc);

	__inline bool IsBadIndex(int index) { return ((index < 0) || (index >= m_numItem)); }
	__inline int GetItemCount() { return (int)m_array.GetCount(); }
	__inline CTabItem* GetItem(int index) { return (IsBadIndex(index) ? NULL : m_array[index]); }
	__inline int GetDefaultMinTabWidth() { return (m_him ? m_sizeIcon.cx : DEFAULT_MIN_TAB_WIDTH); }
	__inline void SetRedraw(BOOL doRedraw) { m_bDoNotRedraw = (doRedraw == FALSE) ? true : false; }
	__inline HWND GetToolTips() { return GetTooltipCtrl(); }
	__inline void SetToolTips(HWND hwndToolTip) { TTAttach(hwndToolTip); }
	__inline LRESULT HitTest(LPTCHITTESTINFO lptest) { return InternalHitTest(lptest->pt, &lptest->flags); }

	DWORD SetExtendedStyle(DWORD mask, DWORD exStyle);
	HFONT GetFont(bool bBold = false);
	HIMAGELIST SetImageList(HIMAGELIST himNew);
	void RecalcTabHeight();
	void SendSelChangeNotify(int indexOld, int indexNew);
	BOOL SendSimpleNotify(UINT code, int index);
	int GetCurFocus();
	BOOL GetItemRect(int index, RECT* prc);
	void InvalidateTabArea();
	int SetCurSel(int index);
	void SetCurFocus(int index);
	void SetPadding(CSize size);
	int InternalHitTest(POINT pt, UINT* flags = NULL);
	int InternalHitTestForDrag(POINT pt);
	void RecalcHotTrack(const POINT* ppt, int* pIndexLeave, int* pIndexEnter);
	void CalcItemBounds();
	void FocusChanging();
	void AdjustRect(BOOL fLarger, LPRECT prc);
	int SetMinTabWidth(int cx);
	void SetFont(HFONT hNewFont);
	int InsertItem(int index, const TCITEM* tcItem);
	BOOL DeleteItem(int index);
	BOOL DeleteAllItems();
	BOOL SetItemT(int index, TCITEM* tcItem);
	BOOL GetItemT(int index, TCITEM* tcItem);
	DWORD GetItemState(int index);
	DWORD SetItemState(int index, DWORD state, DWORD stateMask);
	inline BOOL HighlightItem(int index, BOOL fHighlight) { return (SetItemState(index, (fHighlight ? TCIS_HIGHLIGHTED : 0), TCIS_HIGHLIGHTED) != (DWORD)-1); }
	inline BOOL MarkItem(int index, BOOL fMark) { return (SetItemState(index, (fMark ? TCIS_MARKED : 0), TCIS_MARKED) != (DWORD)-1); }
	BOOL IsMarkedItem(int index);
	LRESULT SetItemSize(CSize size);
};



////////////////////////////////////////////////////////////////////////
//

//#define CLR_STRIPE_1ST			RGB(0xec, 0xec, 0xec)
//#define CLR_STRIPE_2DN			RGB(0xf0, 0xf0, 0xf0)
#define CLR_STRIPE_1ST			RGB(0xec, 0xec, 0xec)
#define CLR_STRIPE_2ND			RGB(0xe2, 0xe2, 0xe2)

int CLccTabCtrl::OnCreate(CREATESTRUCT* pcs)
{
	m_owner = pcs->hwndParent;

	// backpanel bitmap
	CClientDC dc(nullptr);
	HBITMAP hbm = CreateCompatibleBitmap(dc, 8, 8);
	if (hbm) {
		CDC dcw;
		dcw.CreateCompatibleDC(dc);
		HBITMAP hbmOld = dcw.SelectBitmap(hbm);
		CRect rc(0, 0, 8, 2);
		for (int i = 0; i < 2; i++) {
			dcw.FillSolidRect(&rc, CLR_STRIPE_1ST);
			rc.OffsetRect(0, 2);
			dcw.FillSolidRect(&rc, CLR_STRIPE_2ND);
			rc.OffsetRect(0, 2);
		}
		dcw.SelectBitmap(hbmOld);
		m_brPanel.CreatePatternBrush(hbm);
		DeleteObject(hbm);
	}

	// We need to get text information so we need a DC and we need to select a font.
	SetFont(NULL);

	// The tab control always has the WS_CLIPSIBLINGS style. Even
	// if you don't specify it in CreateWindow. This is necessary in
	// order for paint to work correctly. This follows windows behaviour.
	if (!(pcs->style & WS_CLIPSIBLINGS))
		::SetWindowLong(m_hWnd, GWL_STYLE, pcs->style | WS_CLIPSIBLINGS);

	if (pcs->style & TCS_TOOLTIPS) {
		TTEnable(true);
	}

	RecalcTabHeight();
	return 0;
}

void CLccTabCtrl::OnDestroy()
{
	while (GetItemCount() > 0) {
		CTabItem* item = GetItem(0);
		m_array.RemoveAt(0);
		if (item)
			delete item;
	}
}

void CLccTabCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	int indexNew = -1;
	switch (nChar) {
	case VK_LEFT:
		indexNew = m_focusIndex - 1;
		break;
	case VK_RIGHT:
		indexNew = m_focusIndex + 1;
		break;
	}

	/*
	 * If we changed to a valid item, change the selection
	 */
	if (!IsBadIndex(indexNew) && m_focusIndex != indexNew) {
		if (!SendSimpleNotify(TCN_SELCHANGING, indexNew)) {
			int indexOld = m_selIndex;
			m_selIndex = indexNew;
			m_focusIndex = indexNew;
			SendSelChangeNotify(indexOld, indexNew);
			InvalidateTabArea();
		}
	}
}

void CLccTabCtrl::OnLButtonDown(UINT fwKeys, CPoint point)
{
	if (GetStyle() & TCS_FOCUSONBUTTONDOWN) {
		SetFocus();
	}

	UINT flags = 0;
	int indexNew = InternalHitTest(point, &flags);
	if (indexNew != -1) {
		CPoint pt = point;
		ClientToScreen(&pt);

		if (m_selIndex != indexNew) {
			// 新しいインデクス
			if (SendSimpleNotify(TCN_SELCHANGING, indexNew))
				return;

			int indexOld = m_selIndex;
			m_selIndex = indexNew;
			m_focusIndex = indexNew;

			SendSelChangeNotify(indexOld, indexNew);

			InvalidateTabArea();
			UpdateWindow();
		}
	}
}

void CLccTabCtrl::OnLButtonUp(UINT fwKeys, CPoint point)
{
	SendSimpleNotify(NM_CLICK, -1);
}

void CLccTabCtrl::OnLButtonDblClk(UINT fwKeys, CPoint point)
{
	SendSimpleNotify(NM_DBLCLK, -1);
}

void CLccTabCtrl::OnRButtonDown(UINT fwKeys, CPoint point)
{
	SendSimpleNotify(NM_RCLICK, -1);
}

void CLccTabCtrl::OnMouseMove(UINT fwKeys, CPoint point)
{
	int indexLeave;
	int indexEnter;
	RecalcHotTrack(&point, &indexLeave, &indexEnter);
	if (indexLeave != indexEnter) {
		TTActivate(FALSE);
		if (indexEnter >= 0 && TTIsEnabled())
			TTActivate(TRUE);
	}

	if (indexLeave >= 0 || indexEnter >= 0) {
		CClientDC dc(m_hWnd);
		if (indexLeave >= 0)
			DrawItem(dc.m_hDC, indexLeave);
		if (indexEnter >= 0)
			DrawItem(dc.m_hDC, indexEnter);
	}

	if (!m_bTrackMouseEvent) {
		TRACKMOUSEEVENT te;
		te.cbSize = sizeof(te);
		te.dwFlags = TME_LEAVE;
		te.hwndTrack = m_hWnd;
		te.dwHoverTime = HOVER_DEFAULT;
		m_bTrackMouseEvent = (_TrackMouseEvent(&te) != FALSE);
	}
}

void CLccTabCtrl::OnMouseLeave()
{
	if (m_bTrackMouseEvent) {
		if (m_hotIndex >= 0) {
			int index = m_hotIndex;
			m_hotIndex = -1;
			m_hotIndexHitTest = TCHT_NOWHERE;

			CClientDC dc(m_hWnd);
			DrawItem(dc.m_hDC, index);
		}
		m_bTrackMouseEvent = false;
	}
}

void CLccTabCtrl::OnNotifyToolTipText(int id, NMHDR* phdr, LRESULT* pResult)
{
	static TCHAR text[MAX_URL_LENGTH];

	if (TTIsEnabled() && m_hotIndex >= 0) {
		CTabItem* item = GetItem(m_hotIndex);
		NMTTDISPINFO* ptt = (NMTTDISPINFO*)phdr;
		text[0] = 0;

		NMTCGETINFOTIP t;
		t.hdr.hwndFrom = m_hWnd;
		t.hdr.idFrom = GetDlgCtrlID();
		t.hdr.code = TCN_GETINFOTIP;
		t.pszText = text;
		t.cchTextMax = ARRAYSIZE(text);
		t.iItem = m_hotIndex;
		t.lParam = (LPARAM)item->m_param;
		m_owner.SendMessage(WM_NOTIFY, t.hdr.idFrom, (LPARAM)&t);

		if (text[0])
			ptt->lpszText = text;
	}
}

void CLccTabCtrl::DrawInsertMark(CDCHandle dc, int index, BOOL bOn)
{
/* @@
	CRect rc;
	GetItemRect((index == m_numItem) ? index - 1 : index, &rc);
	if (index == m_numItem)
		rc.OffsetRect(rc.Width(), 0);
	rc.left -= 2;
	rc.right = rc.left + 4;
	dc.PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), DSTINVERT);
@@ */

	static const int nFontHeight = 16;
	if (m_fontMarlett.IsNull())
		m_fontMarlett.CreateFont(nFontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, SYMBOL_CHARSET, 0, 0, 0, 0, _T("Marlett"));

	CRect rcc;
	GetClientRect(&rcc);

	CRect rc;
	GetItemRect((index == m_numItem) ? index - 1 : index, &rc);
	if (index == m_numItem)
		rc.OffsetRect(rc.Width(), 0);
	rc.left -= nFontHeight / 2;
	rc.right = rc.left + nFontHeight;
	rc.top = rcc.top;
	rc.bottom = rcc.bottom;

	if (bOn) {
		TCHAR ch = _T('t');
		HFONT hfont = dc.SelectFont(m_fontMarlett);
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(RGB(0xcc, 0, 0));
		dc.DrawText(&ch, 1, &rc, DT_CENTER | DT_TOP | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP);
		ch = _T('u');
		dc.DrawText(&ch, 1, &rc, DT_CENTER | DT_BOTTOM | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP);
		dc.SelectFont(hfont);
	}
	else {
		RedrawWindow(&rc, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

void CLccTabCtrl::DrawItem(CDCHandle dc, int index)
{
	// Get the rectangle for the item.
	CRect rcItem;
	if (!GetItemRect(index, &rcItem)) return;

	CTabItem* item = GetItem(index);

	LONG style  = GetStyle();
	bool bSelected = (index == m_selIndex);
	bool bHot = (index == m_hotIndex);
	bool bHilite = ((item->m_state & TCIS_HIGHLIGHTED) != 0);
	bool bFocus = (!(style & TCS_FOCUSNEVER) && (index == m_focusIndex));

	COLORREF clrLight = GetSysColor(COLOR_3DLIGHT);
	COLORREF clrFace = GetSysColor(COLOR_3DFACE);
	COLORREF clrShadow = GetSysColor(COLOR_3DSHADOW);
	COLORREF clrLtShadow = mix_color(clrShadow, clrFace, 50);
	COLORREF clrDkShadow = GetSysColor(COLOR_3DDKSHADOW);
	COLORREF clrPane = GetSysColor(COLOR_WINDOW);

	CDC dcw;
	dcw.CreateCompatibleDC(dc);
	dcw.SetBkMode(TRANSPARENT);

	HBITMAP hbm = dcw.SelectBitmap(CreateCompatibleBitmap(dc, rcItem.Width(), rcItem.Height()));

	// タブ矩形アウトラインの描画
	CRect rc(0, 0, rcItem.Width(), rcItem.Height());
	dcw.FillSolidRect(&rc, clrLtShadow);

	CRect rci = rc;
	if (style & TCS_BOTTOM)
		rci.top -= 4;					// 見えない部分
	else
		rci.bottom += 4;				// 見えない部分
	COLORREF clr = (bSelected || bHot || bHilite) ? clrDkShadow : clrShadow;
	HPEN hpen = dcw.SelectPen(CreatePen(PS_SOLID, 0, clr));
	HBRUSH hbr = dcw.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
	dcw.RoundRect(rci.left, rci.top, rci.right, rci.bottom, 6, 6);
	DeleteObject(dcw.SelectPen(hpen));

	hpen = dcw.SelectPen(CreatePen(PS_SOLID, 0, RGB(0xff,0xff,0xff)));
	rci.DeflateRect(1, 1);
	dcw.RoundRect(rci.left, rci.top, rci.right, rci.bottom, 4, 4);
	DeleteObject(dcw.SelectPen(hpen));
	dcw.SelectBrush(hbr);
	if (style & TCS_BOTTOM)
		rci.top += 2;
	else
		rci.bottom -= 2;

	// 内部
	rci.DeflateRect(1, 1);

	CRect rcw = rci;
	rcw.bottom = rcw.top + rci.Height() / 2;
	dcw.FillSolidRect(&rcw, (bSelected || bHot || bHilite) ? clrPane : clrFace);
	rcw.top = rcw.bottom;
	rcw.bottom = rci.bottom;
	dcw.FillSolidRect(&rcw, (bSelected || bHot || bHilite) ? clrLight : mix_color(clrShadow, clrFace, 20));

	if (bHilite) {
		Graphics g(dcw);
		COLORREF c = GetSysColor(COLOR_HIGHLIGHT);
		SolidBrush b(Color(80, GetRValue(c), GetGValue(c), GetBValue(c)));
		g.FillRectangle(&b, rci.left, rci.top, rci.Width(), rci.Height());
	}

	// タブ矩形内部の描画
	// アイコン
	rc.SetRect(0, 0, rcItem.Width(), rcItem.Height());
	rc.DeflateRect(m_sizePadding.cx, 0);
	if (m_him && (item->m_iImage >= 0)) {
		ImageList_Draw(m_him, item->m_iImage, dcw, rc.left, (rc.bottom - m_sizeIcon.cy) / 2, ILD_TRANSPARENT);
		rc.left += m_sizeIcon.cx + EXTRA_ICON_PADDING;
	}

	// テキスト
	if (!item->m_strText.IsEmpty()) {
		HFONT hOldFont = dcw.SelectFont(GetFont(bSelected));

		dcw.SetTextColor(GetSysColor(COLOR_BTNTEXT));
		dcw.DrawText(item->m_strText, -1, &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

		dcw.SelectFont(hOldFont);
	}

	// フォーカス矩形
	if (bFocus && (GetFocus() == m_hWnd)) {
		rc.SetRect(0, 0, rcItem.Width(), rcItem.Height());
		rc.InflateRect(-2, -2);
		dcw.DrawFocusRect(&rc);
	}

	// セパレータ
	if (!bSelected) {
		rcw.SetRect(0, 0, rcItem.Width(), rcItem.Height());
		if (style & TCS_BOTTOM)
			rcw.bottom = rcw.top + 1;
		else
			rcw.top = rcw.bottom - 1;
		dcw.FillSolidRect(&rcw, clrDkShadow);
	}

	int nSave = 0;
	CRect rcs;
	if (!bSelected && GetItemRect(m_selIndex, &rcs)) {
		nSave = dc.SaveDC();
		dc.ExcludeClipRect(&rcs);
	}
	dc.BitBlt(rcItem.left, rcItem.top, rcItem.Width(), rcItem.Height(), dcw, 0, 0, SRCCOPY);
	if (nSave)
		dc.RestoreDC(nSave);

	DeleteObject(dcw.SelectBitmap(hbm));
}

void CLccTabCtrl::EraseBkgnd(CDCHandle dc)
{
	COLORREF clrFace = GetSysColor(COLOR_3DFACE);
	COLORREF clrBorder = mix_color(GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DFACE), 50);
	COLORREF clrPane = GetSysColor(COLOR_WINDOW);

	DWORD style = GetStyle();

	int save = dc.SaveDC();
	CRect rc;
	for (int i = 0; i < m_numItem; i++) {
		if (GetItemRect(i, &rc)) {
			if (i == m_selIndex) {
				if (style & TCS_BOTTOM)
					rc.top--;
				else
					rc.bottom++;
			}
			dc.ExcludeClipRect(&rc);
		}
	}

	GetClientRect(&rc);
	if (m_brPanel.IsNull())
		dc.FillSolidRect(&rc, clrFace);
	else {
		HBRUSH hbr = dc.SelectBrush(m_brPanel);
		dc.PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), PATCOPY);
		dc.SelectBrush(hbr);
	}

	if (style & TCS_BOTTOM)
		rc.bottom -= (m_cyTopMargin + m_tabHeight * m_numRows - 1);
	else
		rc.top += (m_cyTopMargin + m_tabHeight * m_numRows - 1);
	dc.FillSolidRect(&rc, clrPane);

	if (style & TCS_BOTTOM)
		rc.top = rc.bottom - 1;
	else
		rc.bottom = rc.top + 1;
	dc.FillSolidRect(&rc, GetSysColor(COLOR_3DDKSHADOW));

	dc.RestoreDC(save);
}


void CLccTabCtrl::OnDraw(CDCHandle dc)
{
	if (m_bDoNotRedraw) return;

	EraseBkgnd(dc);

	int i;
	for (i = 0; i < m_numItem; i++) {
		if (i != m_selIndex)
			DrawItem(dc, i);
	}
	DrawItem(dc, m_selIndex);

	// If we haven't set the current focus yet, set it now.
	// Only happens when we first paint the tab controls
	if (m_focusIndex == -1)
		SetCurFocus(m_selIndex);
}

void CLccTabCtrl::OnSize(UINT nType, int cx, int cy)
{
	if (nType != SIZE_MINIMIZED) {
		CalcItemBounds();
		Invalidate(TRUE);
	}
}

UINT CLccTabCtrl::OnNcCalcSize(BOOL bCalcValidateRect, NCCALCSIZE_PARAMS* pncp)
{
	return (bCalcValidateRect ? 0 : WVR_ALIGNTOP);
}

UINT CLccTabCtrl::OnNcHitTest(int xPos, int yPos)
{
	CPoint pt(xPos, yPos);
	ScreenToClient(&pt);
/* @@
	if (InternalHitTest(pt) == -1)
		return HTTRANSPARENT;
	else
		return HTCLIENT;
@@ */
	// シェルドラッグ＆ドロップの際に、クライアント領域内であればそれを正確に戻さないと
	// Ｄ＆Ｄ対象にならない
	CRect rc;
	GetClientRect(&rc);
	if (rc.PtInRect(pt))
		return HTCLIENT;
	return HTNOWHERE;
}

void CLccTabCtrl::OnStyleChanged(WPARAM, LPARAM)
{
	CalcItemBounds();
	Invalidate(TRUE);
}

void CLccTabCtrl::OnPrintClient(HDC hdc, LPARAM flags)
{
	if (flags & PRF_ERASEBKGND)
		DefWindowProc(WM_ERASEBKGND, (WPARAM)hdc, 0);

	if (flags & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED))
		OnDraw(hdc);
}

void CLccTabCtrl::OnPaint()
{
	CPaintDC dc(m_hWnd);
	OnDraw(dc.m_hDC);
}

DWORD CLccTabCtrl::SetExtendedStyle(DWORD mask, DWORD exStyle)
{
	DWORD old = m_exStyle;
	m_exStyle = ((m_exStyle & ~mask) | exStyle);

	return old;
}

HFONT CLccTabCtrl::GetFont(bool bBold)
{
	ATLASSERT(m_font.m_hFont != NULL);
	ATLASSERT(m_fontBold.m_hFont != NULL);
	return (bBold ? m_fontBold : m_font);
}

HIMAGELIST CLccTabCtrl::SetImageList(HIMAGELIST himNew)
{
	HIMAGELIST himPrev = m_him;
	m_him = himNew;

	int cx = 0, cy = 0;
	if (m_him)
		ImageList_GetIconSize(m_him, &cx, &cy);
	m_sizeIcon = CSize(cx, cy);

	CalcItemBounds();
	InvalidateTabArea();
	return himPrev;
}

void CLccTabCtrl::RecalcTabHeight()
{
	CClientDC dc(m_hWnd);
	// ボールドフォントで計測
	HFONT hfont = dc.SelectFont(GetFont(true));

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	int cy = max(tm.tmHeight, m_sizeIcon.cy);
	m_tabHeight = cy + m_sizePadding.cy * 2;

	dc.SelectFont(hfont);
}

void CLccTabCtrl::SendSelChangeNotify(int indexOld, int indexNew)
{
	NMTCSELCHANGE sc;
	sc.hdr.hwndFrom = m_hWnd;
	sc.hdr.idFrom = GetDlgCtrlID();
	sc.hdr.code = TCN_SELCHANGE;
	sc.iItemOld = indexOld;
	sc.iItemNew = indexNew;
	m_owner.SendMessage(WM_NOTIFY, (WPARAM)sc.hdr.idFrom, (LPARAM)&sc);
}

BOOL CLccTabCtrl::SendSimpleNotify(UINT code, int index)
{
	NMTAB tab;
	memset(&tab, 0, sizeof(tab));
	tab.hdr.hwndFrom = m_hWnd;
	tab.hdr.idFrom = GetDlgCtrlID();
	tab.hdr.code = code;
	tab.iItem = index;
	CTabItem* item = GetItem(index);
	if (item)
		tab.lParam = item->m_param;

	return (BOOL)m_owner.SendMessage(WM_NOTIFY, (WPARAM)tab.hdr.idFrom, (LPARAM)&tab);
}

int CLccTabCtrl::GetCurFocus()
{
	if (m_focusIndex < 0) {
		// FIXME("we have not to return negative value");
		return 0;
	}
	return m_focusIndex;
}

BOOL CLccTabCtrl::GetItemRect(int index, RECT* prc)
{
	// Perform a sanity check and a trivial visibility check.
	if (IsBadIndex(index)) {
		// need to initialize these to empty rects
		if (prc) {
			SetRectEmpty(prc);
			prc->bottom = m_tabHeight;
		}
		return FALSE;
	}

	CTabItem* item = GetItem(index);
	CRect rcItem = item->m_rect;

	CRect rc;
	GetClientRect(&rc);

	LONG style = GetStyle();
	if (style & TCS_BOTTOM) {
		rcItem.OffsetRect(m_cxMargin, rc.Height() - m_cyTopMargin - m_tabHeight);
		rcItem.bottom = rcItem.top + m_tabHeight;
	}
	else {
		rcItem.OffsetRect(m_cxMargin, m_cyTopMargin);
		rcItem.bottom = rcItem.top + m_tabHeight;
	}

	// Check for visibility
	if (prc)
		*prc = rcItem;
	return (rcItem.left < rc.right) && (rcItem.right > rc.left);
}

void CLccTabCtrl::InvalidateTabArea()
{
	if (m_numRows < 1) return;

	if (IsWindowVisible()) {
		CRect rc, rcw;
		GetItemRect(0, &rc);
		GetItemRect(m_numItem - 1, &rcw);

		rc.right = rcw.right;

		LONG style  = GetStyle();
		if (style & TCS_BOTTOM)
			rc.top--;
		else
			rc.bottom++;

		InvalidateRect(&rc, TRUE);
	}
}

int CLccTabCtrl::SetCurSel(int index)
{
	int prevIndex = -1;
	if (!IsBadIndex(index)) {
		prevIndex = m_selIndex;
		m_selIndex = index;
		InvalidateTabArea();
	}
	return prevIndex;
}

void CLccTabCtrl::SetCurFocus(int index)
{
	if (IsBadIndex(index)) return;

	int oldFocus = m_focusIndex;
	if (m_selIndex != index || oldFocus == -1 ) {
		m_focusIndex = index;
		if (oldFocus != -1) {
			if (!SendSimpleNotify(TCN_SELCHANGING, index))  {
				int indexOld = m_selIndex;
				m_selIndex = index;
				SendSelChangeNotify(indexOld, index);
			}
			else
				m_selIndex = index;
			InvalidateTabArea();
		}
	}
}

void CLccTabCtrl::SetPadding(CSize size)
{
	// タブコントロールの各タブのアイコンとラベル周囲のスペース量を指定
	m_sizePadding = size;
}

int CLccTabCtrl::InternalHitTest(POINT pt, UINT* flags)
{
	CRect rc;
	for (int i = 0; i < m_numItem; i++) {
		GetItemRect(i, &rc);
		if (rc.PtInRect(pt)) {
			if (flags) {
				CRect r;
				// まずアイコン位置
				if (m_him) {
					r = rc;
					r.right = rc.left + m_sizePadding.cx + m_sizeIcon.cx + EXTRA_ICON_PADDING;
					if (r.PtInRect(pt)) {
						*flags = TCHT_ONITEMICON;
						return i;
					}
				}

				*flags = TCHT_ONITEMLABEL;
			}
			return i;
		}
	}
	if (flags)
		*flags = TCHT_NOWHERE;
	return -1;
}

int CLccTabCtrl::InternalHitTestForDrag(POINT pt)
{
	CRect rcc;
	GetClientRect(&rcc);
	if (!rcc.PtInRect(pt))
		return -1;

	int i;
	CRect rc;
	for (i = 0; i < m_numItem; i++) {
		GetItemRect(i, &rc);
		rc.top = rcc.top;
		rc.bottom = rcc.bottom;

		if (i == 0) {
			if (pt.x < rc.left)
				break;
		}

		if (rc.PtInRect(pt)) {
			if (pt.x > rc.left + rc.Width() / 2)
				i++;
			break;
		}
	}
	return i;
}

void CLccTabCtrl::RecalcHotTrack(const POINT* ppt, int* pIndexLeave, int* pIndexEnter)
{
	int index = -1;
	if (pIndexLeave) *pIndexLeave = -1;
	if (pIndexEnter) *pIndexEnter = -1;

	CPoint pt;
	if (ppt == NULL) {
		GetCursorPos(&pt);
		ScreenToClient(&pt);
	}
	else
		pt = *ppt;

	UINT hittest = 0;
	index = InternalHitTest(pt, &hittest);

	if (index != m_hotIndex) {
		if (m_hotIndex >= 0) {
			// Mark currently hot-tracked to be redrawn to look normal
			if (pIndexLeave) *pIndexLeave = m_hotIndex;
		}
		m_hotIndex = index;
		if (index >= 0) {
			// Mark new hot-tracked to be redrawn to look highlighted
			if (pIndexEnter) *pIndexEnter = index;
		}
	}
	else if (hittest != m_hotIndexHitTest) {
		m_hotIndexHitTest = hittest;
		if (pIndexEnter) *pIndexEnter = m_hotIndex;
	}
}

void CLccTabCtrl::CalcItemBounds()
{
	if (!m_bHeightSet)
		RecalcTabHeight();

	CClientDC dc(m_hWnd);
	// ボールドフォントで計測
	HFONT hOldFont = dc.SelectFont(GetFont(true));

	int tabMinWidth = (m_tabMinWidth < 0) ? GetDefaultMinTabWidth() : m_tabMinWidth;

	CTabItem* item;
	int cxTab, posLeftSide = 0;
	int i;
	for (i = 0; i < m_numItem; i++) {
		item = GetItem(i);

		// Set the leftmost position of the tab.
		item->m_rect.left = posLeftSide;

		cxTab = 0;
		if (item->m_strText.IsEmpty())
			cxTab += tabMinWidth;
		else {
			// Calculate how wide the tab is depending on the text it contains
			CRect rcText(0, 0, 0, 0);
			dc.DrawText(item->m_strText, -1, &rcText, DT_SINGLELINE | DT_CALCRECT);
			cxTab += rcText.Width() + 2 * m_sizePadding.cx;
			cxTab = max(cxTab, m_tabMinWidth);
		}

		// イメージリストアイコンがある場合
		if (m_sizeIcon.cx > 0)
			cxTab += m_sizeIcon.cx + EXTRA_ICON_PADDING;

		item->m_rect.right = item->m_rect.left + cxTab;
		item->m_rect.bottom = 0;
		item->m_rect.top = 0;

		// The leftmost position of the next item is the rightmost position of this one.
		posLeftSide = item->m_rect.right + m_cxTabSpacing;
	}

	if (m_numItem > 0) {
		m_numRows = 1;

		posLeftSide -= m_cxTabSpacing;		// 余分に足した分を戻す

		CRect rc;
		GetClientRect(&rc);
		if (posLeftSide + 2 * m_cxMargin > rc.Width()) {
			// クライアント矩形の幅より、タブ全体の幅が大きい場合
			// 比率を計算して、各タブを縮小する
			double scale = (double)(rc.Width() - 2 * m_cxMargin) / (double)posLeftSide;

			posLeftSide = 0;
			for (i = 0; i < m_numItem; i++) {
				item = GetItem(i);

				cxTab = item->m_rect.Width();

				// Set the leftmost position of the tab.
				item->m_rect.left = posLeftSide;
				cxTab = FLOOR_INT(cxTab * scale);
				cxTab = max(cxTab, tabMinWidth);

				item->m_rect.right = item->m_rect.left + cxTab;
				posLeftSide = item->m_rect.right + m_cxTabSpacing;
			}
		}
	}
	else
		m_numRows = 0;

	RecalcHotTrack(NULL, NULL, NULL);

	// Cleanup
	dc.SelectFont(hOldFont);
}

void CLccTabCtrl::FocusChanging()
{
	CRect rc;
	if (GetItemRect(m_focusIndex, &rc))
		InvalidateRect(&rc, FALSE);
}

void CLccTabCtrl::AdjustRect(BOOL fLarger, LPRECT prc)
{
	DWORD style = GetStyle();

	int iWholeHeight = m_cyTopMargin + m_tabHeight * m_numRows + m_cyBottomMargin;
	if (fLarger) {	// Go from display rectangle
		// Add the height of the tabs.
		if (style & TCS_BOTTOM)
			prc->bottom += iWholeHeight;
		else
			prc->top -= iWholeHeight;

		InflateRect(prc, m_cxMargin, 0);
	}
    else {			// Go from window rectangle.
		InflateRect(prc, -m_cxMargin, 0);

		// Remove the height of the tabs.
		if (style & TCS_BOTTOM)
			prc->bottom -= iWholeHeight;
		else
			prc->top += iWholeHeight;
	}
}

int CLccTabCtrl::SetMinTabWidth(int cx)
{
	int oldcx = m_tabMinWidth;
	m_tabMinWidth = cx;
	CalcItemBounds();
	return oldcx;
}

void CLccTabCtrl::OnSetFont(WPARAM wParam, LPARAM)
{
	SetFont((HFONT)wParam);
	CalcItemBounds();
	InvalidateTabArea();
}

void CLccTabCtrl::SetFont(HFONT hNewFont)
{
	if (!m_font.IsNull()) m_font.DeleteObject();
	if (!m_fontBold.IsNull()) m_fontBold.DeleteObject();

	LOGFONT lf;
	if (!hNewFont)
		hNewFont = AtlGetStockFont(DEFAULT_GUI_FONT);
	GetObject(hNewFont, sizeof(lf), &lf);

	m_font.CreateFontIndirect(&lf);
	lf.lfWeight = FW_BOLD;
	m_fontBold.CreateFontIndirect(&lf);
}

int CLccTabCtrl::InsertItem(int index, const TCITEM* tcItem)
{
	if (index < 0) return -1;
	if (index > m_numItem)
		index = m_numItem;

	CTabItem* item = new CTabItem;
	if (tcItem->mask & TCIF_TEXT)	item->m_strText = tcItem->pszText;
	if (tcItem->mask & TCIF_IMAGE)	item->m_iImage = tcItem->iImage;
	if (tcItem->mask & TCIF_PARAM)	item->m_param = tcItem->lParam;

	if (m_numItem == 0)
		m_selIndex = 0;
	m_array.InsertAt(index, item);
	m_numItem = GetItemCount();

	CalcItemBounds();
	InvalidateTabArea();
	return index;
}

BOOL CLccTabCtrl::DeleteItem(int index)
{
	if (IsBadIndex(index)) return FALSE;

	int selSave = m_selIndex;				// TCN_DELETEITEMを送った先でm_selIndexが変更されることもある
	SendSimpleNotify(TCN_DELETEITEM, index);

	CTabItem* item = GetItem(index);
	InvalidateTabArea();

	m_array.RemoveAt(index);
	m_numItem = GetItemCount();
	delete item;

	if (selSave == m_selIndex) {
		// Readjust the selected index
		bool bSelChange = false;
		if (index == m_selIndex) {
			bSelChange = true;
			if (index > 0)
				m_selIndex--;
		}
		if (index < m_selIndex)
			m_selIndex--;
		if (m_numItem == 0)
			m_selIndex = -1;
	}

	// Reposition and repaint tabs
	CalcItemBounds();
	return TRUE;
}

BOOL CLccTabCtrl::DeleteAllItems()
{
	while (m_numItem) {
		SendSimpleNotify(TCN_DELETEITEM, 0);
		CTabItem* item = GetItem(0);
		m_array.RemoveAt(0);
		delete item;
		m_numItem--;
	}

	m_selIndex = -1;
	m_focusIndex = -1;
	m_hotIndex = -1;
	m_hotIndexHitTest = TCHT_NOWHERE;

	CalcItemBounds();
	Invalidate(TRUE);

	return TRUE;
}

BOOL CLccTabCtrl::SetItemT(int index, TCITEM* tcItem)
{
	if (IsBadIndex(index)) return FALSE;

	bool recalc = false;
	CTabItem* item = GetItem(index);
	if (tcItem->mask & TCIF_IMAGE)
		item->m_iImage = tcItem->iImage;
	if (tcItem->mask & TCIF_PARAM)
		item->m_param = tcItem->lParam;
	if (tcItem->mask & TCIF_STATE)
		item->m_state = tcItem->dwState;
	if (tcItem->mask & TCIF_TEXT) {
		CString strOld = item->m_strText.GetString();
		item->m_strText = tcItem->pszText;
		// Update and repaint tabs
		if (strOld.Compare(item->m_strText)) {
			recalc = true;
			CalcItemBounds();
		}
	}

	CRect rcc;
	GetClientRect(&rcc);
	CRect rc;
	GetItemRect(index, &rc);

	if (recalc) {
		rcc.left = rc.left;
		rc = rcc;
	}
	InvalidateRect(&rcc, TRUE);
	return TRUE;
}

BOOL CLccTabCtrl::GetItemT(int index, TCITEM* tcItem)
{
	if (IsBadIndex(index)) return FALSE;

	CTabItem* item = GetItem(index);
	if (tcItem->mask & TCIF_IMAGE)	tcItem->iImage = item->m_iImage;
	if (tcItem->mask & TCIF_PARAM)	tcItem->lParam = item->m_param;
	if (tcItem->mask & TCIF_STATE)	tcItem->dwState = item->m_state;
	if (tcItem->mask & TCIF_TEXT)	lstrcpyn(tcItem->pszText, item->m_strText, tcItem->cchTextMax);

	return TRUE;
}

DWORD CLccTabCtrl::GetItemState(int index)
{
	if (IsBadIndex(index)) return (DWORD)-1;

	CTabItem* item = GetItem(index);
	return item->m_state;
}

DWORD CLccTabCtrl::SetItemState(int index, DWORD state, DWORD stateMask)
{
	if (IsBadIndex(index)) return (DWORD)-1;

	CTabItem* item = GetItem(index);
	DWORD newState = (item->m_state & ~stateMask) | state;

	if (item->m_state != newState) {
		item->m_state = newState;
		CRect rc;
		if (GetItemRect(index, &rc))
			InvalidateRect(&rc, FALSE);
	}
	return newState;
}

BOOL CLccTabCtrl::IsMarkedItem(int index)
{
	DWORD state = GetItemState(index);
	if (state != (DWORD)-1)
		return ((state & TCIS_MARKED) != 0);
	return FALSE;
}

LRESULT CLccTabCtrl::SetItemSize(CSize size)
{
	LONG lResult = MAKELONG(0, m_tabHeight);
	if (m_tabHeight != size.cy) {
		m_bHeightSet = (size.cy != 0);
		if (m_bHeightSet)
			m_tabHeight = size.cy;

		CalcItemBounds();
		RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
	}
	return lResult;
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK TabWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CLccTabCtrl* pWnd = new CLccTabCtrl;
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}

