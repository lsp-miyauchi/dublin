/* Tab_ex control
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
#define ROUND_INT(n)					((int)floor((n)+0.5F))

////////////////////////////////////////////////////////////////////////
// Positioning constants

#define DEFAULT_MIN_TAB_WIDTH			30
#define PANEL_FIN						15
#define EXTRA_ICON_PADDING				2					// アイコン－テキスト間のパディング
#define EXTRA_CLOSE_PADDING				12					// テキスト－クローズボタン間のパディング
#define MAX_URL_LENGTH					(2048 + 32 + 8)
#define CXSPACER						3					// 標準のスペーサー


////////////////////////////////////////////////////////////////////////
//

class CTabItem
{
public:
	DWORD m_state;
	CString m_strText;
	int m_iImage;
	COLORREF m_clrText;
	LPARAM m_param;
	CRect m_rect;

public:
	CTabItem()
		: m_state(0)
		, m_iImage(-1)
		, m_clrText(GetSysColor(COLOR_BTNTEXT))
		, m_param(0)
	{
	}
	CTabItem(const CTabItem& src)
	{
		CopyFrom(src);
	}

	~CTabItem()
	{
	}

	CTabItem& CopyFrom(const CTabItem& src)
	{
		if (this != &src) {
			m_state = src.m_state;
			m_strText = src.m_strText;
			m_iImage = src.m_iImage;
			m_clrText = src.m_clrText;
			m_param = src.m_param;
			m_rect = src.m_rect;
		}
		return (*this);
	}

	inline CTabItem& operator=(const CTabItem& src) { return CopyFrom(src); }

	inline bool isMarked() { return ((m_state & TCIS_MARKED) != 0); }
	inline bool isLocked() { return ((m_state & TCIS_LOCKED) != 0); }
};

////////////////////////////////////////////////////////////////////////
//

static void GdipDrawString(Graphics* pgr, Font* pft, LPCTSTR psz, RECT* prc, UINT flg, COLORREF clr)
{
	StringFormat fmt;
	if      (flg & DT_SINGLELINE)		fmt.SetFormatFlags(StringFormatFlagsNoWrap);		// DT_SINGLELINE

	if      (flg & DT_CENTER)			fmt.SetAlignment(StringAlignmentCenter);			// DT_CENTER
	else if (flg & DT_RIGHT)			fmt.SetAlignment(StringAlignmentFar);				// DT_RIGHT
	else								fmt.SetAlignment(StringAlignmentNear);				// DT_LEFT

	if      (flg & DT_PATH_ELLIPSIS)	fmt.SetTrimming(StringTrimmingEllipsisPath);		// DT_PATH_ELLIPSIS
	else if (flg & DT_WORD_ELLIPSIS)	fmt.SetTrimming(StringTrimmingEllipsisWord);		// DT_WORD_ELLIPSIS
	else if (flg & DT_END_ELLIPSIS)		fmt.SetTrimming(StringTrimmingEllipsisCharacter);	// DT_END_ELLIPSIS

	if      (flg & DT_VCENTER)			fmt.SetLineAlignment(StringAlignmentCenter);		// DT_VCENTER
	else if (flg & DT_BOTTOM)			fmt.SetLineAlignment(StringAlignmentFar);			// DT_BOTTOM
	else								fmt.SetLineAlignment(StringAlignmentNear);			// DT_TOP

	if      (flg & DT_NOPREFIX)			fmt.SetHotkeyPrefix(HotkeyPrefixNone);				// DT_NOPREFIX
	else if (flg & DT_HIDEPREFIX)		fmt.SetHotkeyPrefix(HotkeyPrefixHide);				// DT_HIDEPREFIX
	else								fmt.SetHotkeyPrefix(HotkeyPrefixShow);

	RectF r((REAL)prc->left, (REAL)prc->top, (REAL)(prc->right - prc->left), (REAL)(prc->bottom - prc->top));
	SolidBrush sbr(Color(255, GetRValue(clr), GetGValue(clr), GetBValue(clr)));
	pgr->SetTextRenderingHint(TextRenderingHintAntiAlias);
	pgr->DrawString(psz, lstrlen(psz), pft, r, &fmt, &sbr);
}

static void DrawString(CDC& dc, HFONT hFont, LPCTSTR psz, RECT* prc, UINT flg, COLORREF clr)
{
	HFONT hf = dc.SelectFont(hFont);
	COLORREF rgb = dc.SetTextColor(clr);
	int bmd = dc.SetBkMode(TRANSPARENT);
	DRAWTEXTPARAMS dtp = {0};
	dtp.cbSize = sizeof(dtp);
	dc.DrawTextEx((LPTSTR)psz, -1, prc, flg, &dtp);
	dc.SetTextColor(rgb);
	dc.SelectFont(hf);
	dc.SetBkMode(bmd);
}

////////////////////////////////////////////////////////////////////////
//

class CLccTabExCtrl	: public CWindowImpl<CLccTabExCtrl>
					, public CToolTipUsing<CLccTabExCtrl>
//@					, public CThemeImpl<CLccTabExCtrl>
{
private:
	// gdi+
	gdiplus_using m_gdiplus_using;

	CWindow m_owner;				// notification window (parent)
	int m_tabHeight;				// height of the tab row
	int m_tabMinWidth;				// タブアイテムに設定する最小幅。この値を -1 にすると、既定のタブ幅が使わる
	int m_dragSourceIndex;
	int m_dragTargetIndex;
	CFont m_fontMarlett;
	float m_cyChar;
	Font* m_pFont;
	Font* m_pFontBold;
	CFont m_font;					// GDIフォント
	CFont m_fontBold;				// 
	int m_selIndex;					// the currently selected item
	int m_hotIndex;					// the highlighted item under the mouse
	int m_focusIndex;				// item which has the focus
	UINT m_hotIndexHitTest;
	CAtlArray<CTabItem> m_array;	// an array of CTabItem's
	int m_cxTabSpacing;				// タブ間スペーシング（マイナス値が重なりサイズを示す）
	int m_cxBarPaddingLeft;			// バー左側のパディング（最初のタブまでのスペース）
	int m_cxBarPaddingRight;		
	CSize m_sizeIcon;
	CSize m_sizeClose;
	CSize m_sizeAdd;
	CSize m_sizeTab;				// パネルイメージリストのアイコンサイズ
	DWORD m_exStyle;				// 拡張スタイル
	CImageList m_imIcon;			// イメージリスト
	CImageList m_imDrag;
	CImageList m_imClose;			// クローズボタン用イメージリスト
	CImageList m_imLock;			// ロックマーク用イメージリスト
	CImageList m_imPanel;			// タブパネルイメージリスト
	CImageList m_imAdd;				// タブパネル追加ボタンイメージリスト
	int m_yDrag;					// ドラッグ中の固定y座標
	bool m_bDoNotRedraw:1;			// flag for redrawing when tab contents is changed
	bool m_bHeightSet:1;			// was the height of the tabs explicitly set?
	bool m_bTrackMouseEvent:1;
	bool m_bDragging:1;				// ドラッグ中
	bool m_bCloseButtonPressed:1;	// 「閉じる」ボタン押下中
	bool m_bDragCursorIn:1;
	bool m_bInAddButton:1;			// 追加ボタン
	bool m_bAddButtonPressed:1;		// 追加ボタン押下中
	bool m_bUseGdipDrawing:1;

	CBrush m_brPanel;
	COLORREF m_clrPane;
	CRect m_rcAddButton;
	CBitmapHandle m_bmBkgnd;
	CBitmap m_bmBkgndCache;

public:
	CLccTabExCtrl()
		: m_tabHeight(0)
		, m_tabMinWidth(-1)
		, m_yDrag(-1)
		, m_selIndex(-1)
		, m_hotIndex(-1)
		, m_focusIndex(-1)
		, m_hotIndexHitTest(TCHT_NOWHERE)
		, m_cxTabSpacing(0)
		, m_cxBarPaddingLeft(0)
		, m_cxBarPaddingRight(0)
		, m_sizeIcon(CSize(0, 0))
		, m_sizeClose(CSize(0, 0))
		, m_exStyle(0)
		, m_dragSourceIndex(-1)
		, m_dragTargetIndex(-1)
		, m_bDoNotRedraw(false)
		, m_bHeightSet(false)
		, m_bTrackMouseEvent(false)
		, m_bDragging(false)
		, m_bCloseButtonPressed(false)
		, m_bDragCursorIn(false)
		, m_bInAddButton(false)
		, m_bAddButtonPressed(false)
		, m_bUseGdipDrawing(false)
		, m_clrPane(GetSysColor(COLOR_3DFACE))
		, m_pFont(NULL)
		, m_pFontBold(NULL)
	{
//@		SetThemeClassList(L"Status");
		CClientDC dc(NULL);
		m_cyChar = 9.0F * dc.GetDeviceCaps(LOGPIXELSY) / 72.0F;
	}

	~CLccTabExCtrl()
	{
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

BEGIN_MSG_MAP(CLccTabExCtrl)
//@	CHAIN_MSG_MAP(CThemeImpl<CLccTabExCtrl>)
	CHAIN_MSG_MAP(CToolTipUsing<CLccTabExCtrl>)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_NCCALCSIZE()
	ON_WM_GETDLGCODE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE_VOID(WM_SETREDRAW, OnSetRedraw)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
	ON_MESSAGE_VOID(WM_CAPTURECHANGED, OnCaptureChanged)
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
	ON_MESSAGE(TCM_REMOVEIMAGE, OnTcmRemoveImage)
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
	ON_MESSAGE(TCM_GETPANECOLOR, OnTcmGetPaneColor)
	ON_MESSAGE(TCM_SETPANECOLOR, OnTcmSetPaneColor)
	ON_MESSAGE(TCM_CREATEDRAGIMAGE, OnTcmCreateDragImage)
	ON_MESSAGE(TCM_TABMOVETO, OnTcmTabMoveTo)
	ON_MESSAGE(TCM_TABEXCHANGE, OnTcmTabExchange)
	ON_MESSAGE_VOID(TCM_SETPARENT, OnTcmSetParent)
	ON_MESSAGE_VOID(TCM_GETBARMETRICS, OnTcmGetBarMetrics)
	ON_MESSAGE_VOID(TCM_SETBARMETRICS, OnTcmSetBarMetrics)
	ON_MESSAGE_VOID(TCM_SETBARMETRICS, OnTcmSetBarMetrics)
	ON_MESSAGE(TCM_GETBKGNDIMAGE, OnTcmGetBackgroundImage)
	ON_MESSAGE(TCM_SETBKGNDIMAGE, OnTcmSetBackgroundImage)
END_MSG_MAP()

private:
	int OnCreate(CREATESTRUCT* pcs);
	void OnDestroy();
	UINT OnNcCalcSize(BOOL bCalcValidateRect, NCCALCSIZE_PARAMS* pncp);
	__inline UINT OnGetDlgCode() { return (GetStyle() & TCS_FOCUSNEVER) ? DLGC_STATIC : (DLGC_WANTARROWS | DLGC_WANTTAB); }
	void OnSize(UINT type, int cx, int cy);
	BOOL OnEraseBkgnd(HDC hdc);
	__inline void OnSetRedraw(WPARAM wParam, LPARAM lParam) { SetRedraw((BOOL)wParam); }
	void OnLButtonDown(UINT nFlags, CPoint pt);
	void OnLButtonUp(UINT nFlags, CPoint pt);
	void OnLButtonDblClk(UINT nFlags, CPoint pt);
	void OnRButtonDown(UINT nFlags, CPoint pt);
	void OnMouseMove(UINT nFlags, CPoint pt);
	void OnMouseLeave();
	LRESULT OnMouseHover(WPARAM wParam, LPARAM lParam);
	void OnCaptureChanged(WPARAM, LPARAM);
	void OnNotifyToolTipText(int id, NMHDR* phdr, LRESULT* pResult);
	void OnSetFocus(WPARAM, LPARAM);
	void OnKillFocus(WPARAM, LPARAM);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	UINT OnNcHitTest(int xPos, int yPos);
	void OnStyleChanged(WPARAM, LPARAM);
	void OnPrintClient(HDC hdc, LPARAM flags);
	void OnPaint();
	__inline LRESULT OnTcmGetImageList(WPARAM, LPARAM) { return (LRESULT)m_imIcon.m_hImageList; }
	__inline LRESULT OnTcmSetImageList(WPARAM, LPARAM lParam) { return (LRESULT)SetImageList((HIMAGELIST)lParam); }
	__inline LRESULT OnTcmGetItemCount(WPARAM, LPARAM) { return (LRESULT)m_array.GetCount(); }
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
	__inline LRESULT OnTcmRemoveImage(WPARAM wParam, LPARAM lParam) { return 0; }
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
	__inline LRESULT OnTcmTabMoveTo(WPARAM wParam, LPARAM lParam) { return (LRESULT)TabMoveTo((int)wParam, (int)lParam); }
	__inline LRESULT OnTcmTabExchange(WPARAM wParam, LPARAM lParam) { return (LRESULT)TabExchange((int)wParam, (int)lParam); }
	__inline void OnTcmSetParent(WPARAM wParam, LPARAM) { ATLASSERT(::IsWindow((HWND)wParam)); m_owner = (HWND)wParam; }
	__inline LRESULT OnTcmGetPaneColor(WPARAM, LPARAM) { return (LRESULT)m_clrPane; }
	__inline LRESULT OnTcmSetPaneColor(WPARAM, LPARAM lParam) { return (LRESULT)SetPaneColor((COLORREF)lParam); }
	__inline LRESULT OnTcmCreateDragImage(WPARAM wParam, LPARAM lParam) { return (LRESULT)CreateDragImage((int)wParam, (POINT*)lParam); }
	__inline void OnTcmGetBarMetrics(WPARAM, LPARAM lParam) { GetBarMetrics((NMTCMETRICS*)lParam); }
	__inline void OnTcmSetBarMetrics(WPARAM, LPARAM lParam) { SetBarMetrics((NMTCMETRICS*)lParam); }
	__inline LRESULT OnTcmGetBackgroundImage(WPARAM, LPARAM) { return (LRESULT)m_bmBkgnd.m_hBitmap; }
	LRESULT OnTcmSetBackgroundImage(WPARAM wParam, LPARAM lParam);

	void EraseBkgnd(HDC hdc, bool bExcludeTabArea);
	void OnDraw(CDCHandle dc);
	void DrawInsertMark(CDCHandle dc, int index, BOOL bOn);
	void DrawItem(CDC& dc, const CRect& rcItem, int index);

	__inline bool isBadIndex(int index) { return ((index < 0) || (index >= (int)m_array.GetCount())); }
	__inline void SetRedraw(BOOL doRedraw) { m_bDoNotRedraw = (doRedraw == FALSE) ? true : false; }
	__inline HWND GetToolTips() { return GetTooltipCtrl(); }
	__inline void SetToolTips(HWND hwndToolTip) { TTAttach(hwndToolTip); }
	__inline LRESULT HitTest(LPTCHITTESTINFO lptest) { return InternalHitTest(lptest->pt, &lptest->flags); }

	int GetMinTabWidth();

	DWORD SetExtendedStyle(DWORD mask, DWORD exStyle);
	Font* GetFont(bool bBold = false);
	HFONT GetFontHandle(bool bBold = false);
	HIMAGELIST SetImageList(HIMAGELIST himNew);
	COLORREF SetPaneColor(COLORREF clr);
	void RecalcTabHeight();
	void SendSelChangeNotify(int indexOld, int indexNew);
	BOOL SendSimpleNotify(UINT code, int index);
	int GetCurFocus();
	BOOL GetItemRect(int index, RECT* prc);
	void GetTabAreaRect(RECT* prc);
	void InvalidateTabArea();
	void GetAddButtonRect(RECT* prc);
	int SetCurSel(int index);
	void SetCurFocus(int index);
	int InternalHitTest(POINT pt, UINT* flags = NULL);
	int InternalHitTestForDrag(POINT pt);
	void RecalcHotTrack(const POINT* ppt, int* pIndexLeave, int* pIndexEnter);
	void CalcItemBounds();
	void FocusChanging();
	void AdjustRect(BOOL fLarger, LPRECT prc);
	int SetMinTabWidth(int cx);
	void GetBarMetrics(NMTCMETRICS* ptcm);
	void SetBarMetrics(NMTCMETRICS* ptcm);
	int InsertItem(int index, const TCITEM* tcItem);
	BOOL DeleteItem(int index);
	BOOL DeleteAllItems();
	BOOL SetItemT(int index, TCITEM* tcItem);
	BOOL GetItemT(int index, TCITEM* tcItem);
	DWORD GetItemState(int index);
	DWORD SetItemState(int index, DWORD state, DWORD stateMask);
	inline BOOL HighlightItem(int index, BOOL fHighlight) { return (SetItemState(index, (fHighlight ? TCIS_HIGHLIGHTED : 0), TCIS_HIGHLIGHTED) != (DWORD)-1); }
	inline BOOL MarkItem(int index, BOOL fMark) { return (SetItemState(index, (fMark ? TCIS_MARKED : 0), TCIS_MARKED) != (DWORD)-1); }
	inline BOOL LockItem(int index, BOOL fLock) { return (SetItemState(index, (fLock ? TCIS_LOCKED : 0), TCIS_LOCKED) != (DWORD)-1); }
	BOOL isStateItem(int index, DWORD state);
	inline BOOL isMarkedItem(int index) { return isStateItem(index, TCIS_MARKED); }
	inline BOOL isLockedItem(int index) { return isStateItem(index, TCIS_LOCKED); }
	HBITMAP CreateDragImage(int index, POINT* ppt);
	bool TabMoveTo(int indexTarget, int indexSource);
	bool TabExchange(int index1, int index2);
};



////////////////////////////////////////////////////////////////////////
//

//#define CLR_STRIPE_1ST			RGB(0xec, 0xec, 0xec)
//#define CLR_STRIPE_2DN			RGB(0xf0, 0xf0, 0xf0)
#define CLR_STRIPE_1ST			RGB(0xf1, 0xed, 0xed)
#define CLR_STRIPE_2ND			RGB(0xe9, 0xe5, 0xe5)

int CLccTabExCtrl::OnCreate(CREATESTRUCT* pcs)
{
	m_owner = pcs->hwndParent;

	// backpanel bitmap
	CClientDC dc(NULL);
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

	// The tab control always has the WS_CLIPSIBLINGS style. Even
	// if you don't specify it in CreateWindow. This is necessary in
	// order for paint to work correctly. This follows windows behaviour.
	if (!(pcs->style & WS_CLIPSIBLINGS))
		::SetWindowLong(m_hWnd, GWL_STYLE, pcs->style | WS_CLIPSIBLINGS);

	if (pcs->style & TCS_TOOLTIPS) {
		TTEnable(true);
	}

	m_sizeClose = CSize(16, 16);
	m_imClose = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_TABCLOSE), m_sizeClose.cx, RGB(255, 0, 255));
	m_imLock = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_TABLOCK), m_sizeClose.cx, RGB(255, 0, 255));

	m_sizeTab = CSize(15, 36);
	m_imPanel = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_TABPANEL), m_sizeTab.cx, RGB(255, 0, 255));
//	m_cxTabSpacing = -m_sizeTab.cx;
	m_cxTabSpacing = -8;

	m_sizeAdd = CSize(20, 20);
	m_imAdd = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_TABADD), m_sizeAdd.cx, RGB(255, 0, 255));

	RecalcTabHeight();
	return 0;
}

void CLccTabExCtrl::OnDestroy()
{
}

void CLccTabExCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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
	if (!isBadIndex(indexNew) && m_focusIndex != indexNew) {
		if (!SendSimpleNotify(TCN_SELCHANGING, indexNew)) {
			int indexOld = m_selIndex;
			m_selIndex = indexNew;
			m_focusIndex = indexNew;
			SendSelChangeNotify(indexOld, indexNew);
			InvalidateTabArea();
		}
	}
}

void CLccTabExCtrl::OnLButtonDown(UINT fwKeys, CPoint point)
{
	if (GetStyle() & TCS_FOCUSONBUTTONDOWN) {
		SetFocus();
	}

	CRect rc;
	CPoint pt = point;
	ClientToScreen(&pt);

	UINT flags = 0;
	int indexNew = InternalHitTest(point, &flags);
	if (indexNew != -1) {
		if (flags & TCHT_ONITEMCLOSE) {
			if (isLockedItem(indexNew)) {
				;
			} else {
				// 閉じるボタン上
				GetItemRect(indexNew, &rc);

				m_bCloseButtonPressed = true;
				RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

				BOOL bClose = !lccBeginTrack(m_hWnd, pt);

				m_bCloseButtonPressed = false;
				RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

				if (bClose) {
					SendSimpleNotify(TCN_CLOSEBUTTONCLICKED, indexNew);
				}
			}
			return;
		}

		// DRAG & DROP
		if ((m_exStyle & TCS_EX_DRAGDROP) && (m_array.GetCount() > 1) && lccBeginTrack(m_hWnd, pt)) {
			if (SendSimpleNotify(TCN_BEGINDRAG, indexNew)) {
				SetCapture();
				m_bDragging = true;
				m_dragSourceIndex = indexNew;

				// 現在カーソル位置を渡して、ビットマップとオフセットを得る
				pt = point;
				CBitmap bmD = CreateDragImage(m_dragSourceIndex, &pt);
				if (!bmD.IsNull()) {
					m_imDrag.Destroy();

					BITMAP bm;
					bmD.GetBitmap(bm);
					m_imDrag.Create(bm.bmWidth, bm.bmHeight, ILC_COLOR24 | ILC_MASK, 1, 1);
					COLORREF clrKey = GetSysColor(COLOR_WINDOW);
					m_imDrag.SetBkColor(clrKey);
					m_imDrag.Add(bmD, clrKey);
					m_imDrag.BeginDrag(0, pt);

					m_yDrag = point.y;
					ImageList_DragEnter(m_hWnd, point.x, point.y);
					m_bDragCursorIn = true;
				}
				return;
			}
		}

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
	} else if (flags == TCHT_ONADDBUTTON) {
		GetAddButtonRect(&rc);

		m_bAddButtonPressed = true;
		RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

		BOOL bAdd = !lccBeginTrack(m_hWnd, pt);

		m_bAddButtonPressed = false;
		RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

		if (bAdd) {
			SendSimpleNotify(TCN_ADDBUTTONCLICKED, 0);
		}
	}
}

bool CLccTabExCtrl::TabMoveTo(int indexTarget, int indexSource)
{
	bool bChanged = false;
	int numTabs = (int)m_array.GetCount();
	if ((indexTarget >= 0 && indexTarget <= numTabs) &&
		(indexSource >= 0 && indexSource < numTabs) &&
		(indexTarget != indexSource) && 
		(indexTarget != indexSource + 1)) {

		MarkItem(m_selIndex, TRUE);		// 目印
		CTabItem item(m_array[indexSource]);
		m_array.RemoveAt(indexSource);

		if (indexTarget < indexSource) {
			m_array.InsertAt(indexTarget, item);
		} else {
			indexTarget--;
			m_array.InsertAt(indexTarget, item);
		}

		int i;
		for (i = 0; i < numTabs; i++) {
			if (isMarkedItem(i)) {
				MarkItem(i, FALSE);
				m_selIndex = i;
				m_focusIndex = i;
				break;
			}
		}
		CalcItemBounds();
		InvalidateTabArea();
		bChanged = true;
	}
	return bChanged;
}

bool CLccTabExCtrl::TabExchange(int index1, int index2)
{
	bool bChanged = false;
	int numTabs = (int)m_array.GetCount();
	if ((index1 >= 0 && index1 <= numTabs) &&
		(index2 >= 0 && index2 <= numTabs) &&
		(index1 != index2)) {

		MarkItem(m_selIndex, TRUE);		// 目印
		CTabItem item(m_array[index2]);
		m_array[index2] = m_array[index1];
		m_array[index1] = item;
		int i;
		for (i = 0; i < numTabs; i++) {
			if (isMarkedItem(i)) {
				MarkItem(i, FALSE);
				m_selIndex = i;
				m_focusIndex = i;
				break;
			}
		}
		CalcItemBounds();
		InvalidateTabArea();
		bChanged = true;
	}
	return bChanged;
}

void CLccTabExCtrl::OnLButtonUp(UINT fwKeys, CPoint point)
{
	if (m_bDragging) {
		if (!m_imDrag.IsNull()) {
			if (m_bDragCursorIn)
				ImageList_DragLeave(m_hWnd);
			ImageList_EndDrag();
			m_imDrag.Destroy();
		}
		m_bDragCursorIn = false;
		m_bDragging = false;
		ReleaseCapture();

		if (m_dragTargetIndex >= 0) {
			CClientDC dc(m_hWnd);
			DrawInsertMark(dc.m_hDC, m_dragTargetIndex, FALSE);

			if (TabMoveTo(m_dragTargetIndex, m_dragSourceIndex))
				SendSimpleNotify(TCN_ENDDRAG, -1);

			m_dragSourceIndex = -1;
			m_dragTargetIndex = -1;
		}
	} else {
		SendSimpleNotify(NM_CLICK, -1);
	}
}

void CLccTabExCtrl::OnCaptureChanged(WPARAM, LPARAM)
{
	if (m_bDragging) {
		m_bDragging = false;

		if (m_dragTargetIndex >= 0) {
			CClientDC dc(m_hWnd);
			DrawInsertMark(dc.m_hDC, m_dragTargetIndex, FALSE);
		}

		m_dragSourceIndex = -1;
		m_dragTargetIndex = -1;
	}
}

void CLccTabExCtrl::OnLButtonDblClk(UINT fwKeys, CPoint point)
{
	UINT flags = 0;
	int index = InternalHitTest(point, &flags);
	SendSimpleNotify(NM_DBLCLK, index);
}

void CLccTabExCtrl::OnRButtonDown(UINT fwKeys, CPoint point)
{
	if (m_bDragging) return;

	SendSimpleNotify(NM_RCLICK, -1);
}

void CLccTabExCtrl::OnMouseMove(UINT fwKeys, CPoint point)
{
	if (m_bDragging) {
		if (!m_imDrag.IsNull()) {
			CPoint ptw = point;
			ClientToScreen(&ptw);
			HWND hwnd = WindowFromPoint(ptw);
			if (hwnd != m_hWnd) {
				if (m_bDragCursorIn) {
					ImageList_DragLeave(m_hWnd);
					m_bDragCursorIn = false;
				}
			} else {
				if (!m_bDragCursorIn) {
					ImageList_DragEnter(m_hWnd, point.x, m_yDrag);
					m_bDragCursorIn = true;
				} else {
					ImageList_DragMove(point.x, m_yDrag);
				}
			}
		}

		int index = InternalHitTestForDrag(point);
		if (index != m_dragTargetIndex) {
			if (m_bDragCursorIn) ImageList_DragShowNolock(FALSE);

			CClientDC dc(m_hWnd);

			if (m_dragTargetIndex >= 0)
				DrawInsertMark(dc.m_hDC, m_dragTargetIndex, FALSE);
			if (index >= 0)
				DrawInsertMark(dc.m_hDC, index, TRUE);
			m_dragTargetIndex = index;

			if (m_bDragCursorIn) ImageList_DragShowNolock(TRUE);

			::SetCursor(::LoadCursor(NULL, (m_dragTargetIndex >= 0) ? IDC_ARROW : IDC_NO));
		}
		return;
	}

	int indexLeave;
	int indexEnter;
	bool bInAddButton = m_bInAddButton;					// 追加ボタンの状態を保存
	RecalcHotTrack(&point, &indexLeave, &indexEnter);
	if (indexLeave != indexEnter) {
		TTActivate(FALSE);
		if (indexEnter >= 0 && TTIsEnabled())
			TTActivate(TRUE);
	}

	CRect rc;
	if (indexLeave >= 0 || indexEnter >= 0) {
		if (indexLeave >= 0) {
			GetItemRect(indexLeave, &rc);
			RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		if (indexEnter >= 0) {
			GetItemRect(indexEnter, &rc);
			RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}

	if (bInAddButton != m_bInAddButton) {
		GetAddButtonRect(&rc);
		RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	if (!m_bTrackMouseEvent) {
		TRACKMOUSEEVENT te;
		te.cbSize = sizeof(te);
		te.dwFlags = TME_LEAVE | TME_HOVER;
		te.dwHoverTime = HOVER_DEFAULT;
		te.hwndTrack = m_hWnd;
		m_bTrackMouseEvent = (_TrackMouseEvent(&te) != FALSE);
	}
}

void CLccTabExCtrl::OnMouseLeave()
{
	if (m_bTrackMouseEvent) {
		CRect rc;
		if (m_hotIndex >= 0) {
			int index = m_hotIndex;
			m_hotIndex = -1;
			m_hotIndexHitTest = TCHT_NOWHERE;

			GetItemRect(index, &rc);
			RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		if (m_bInAddButton) {
			m_bInAddButton = false;
			GetAddButtonRect(&rc);
			RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		m_bTrackMouseEvent = false;
	}
}

LRESULT CLccTabExCtrl::OnMouseHover(WPARAM wParam, LPARAM lParam)
{
	if (m_bTrackMouseEvent &&
		(m_exStyle & TCS_EX_HOVERSELECT) &&
		(GetActiveWindow() == gettopparent(m_hWnd)) &&
		!m_bDragging &&
		(m_hotIndex >= 0) &&
		(m_selIndex != m_hotIndex))
	{
		// 新しいインデクス
		if (!SendSimpleNotify(TCN_SELCHANGING, m_hotIndex)) {
			int indexOld = m_selIndex;
			m_selIndex = m_hotIndex;
			m_focusIndex = m_hotIndex;
			SendSelChangeNotify(indexOld, m_hotIndex);

			InvalidateTabArea();
			UpdateWindow();
		}
	}
	m_bTrackMouseEvent = FALSE;
	return 0;
}

void CLccTabExCtrl::OnNotifyToolTipText(int id, NMHDR* phdr, LRESULT* pResult)
{
	static TCHAR text[MAX_URL_LENGTH];

	if (TTIsEnabled() && m_hotIndex >= 0) {
		CTabItem& item = m_array[m_hotIndex];
		NMTTDISPINFO* ptt = (NMTTDISPINFO*)phdr;
		text[0] = 0;

		NMTCGETINFOTIP t;
		t.hdr.hwndFrom = m_hWnd;
		t.hdr.idFrom = GetDlgCtrlID();
		t.hdr.code = TCN_GETINFOTIP;
		t.pszText = text;
		t.cchTextMax = ARRAYSIZE(text);
		t.iItem = m_hotIndex;
		t.lParam = (LPARAM)item.m_param;
		m_owner.SendMessage(WM_NOTIFY, t.hdr.idFrom, (LPARAM)&t);

		if (text[0])
			ptt->lpszText = text;
	}
}

void CLccTabExCtrl::DrawInsertMark(CDCHandle dc, int index, BOOL bOn)
{
	static const int nFontHeight = 16;
	if (m_fontMarlett.IsNull())
		m_fontMarlett.CreateFont(nFontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, SYMBOL_CHARSET, 0, 0, 0, 0, _T("Marlett"));

	int numItems = (int)m_array.GetCount();
	CRect rc;
	GetItemRect((index == numItems) ? index - 1 : index, &rc);
	if (index == numItems)
		rc.OffsetRect(rc.Width() + m_cxTabSpacing, 0);
	rc.left -= nFontHeight / 2 + (m_cxTabSpacing / 2);
	rc.right = rc.left + nFontHeight;

	if (bOn) {
		TCHAR ch = _T('6');
		HFONT hfont = dc.SelectFont(m_fontMarlett);
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(RGB(0xcc, 0, 0));
		dc.DrawText(&ch, 1, &rc, DT_CENTER | DT_TOP | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP);
	} else {
		RedrawWindow(&rc, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

void CLccTabExCtrl::EraseBkgnd(HDC hdc, bool bExcludeTabArea)
{
	CDCHandle dc = hdc;

	int save = dc.SaveDC();

	// タブ領域を除く
	CRect rc;
	GetTabAreaRect(&rc);
	if (bExcludeTabArea && !rc.IsRectEmpty()) {
		dc.ExcludeClipRect(&rc);
		// 追加ボタン領域を除く
		GetAddButtonRect(&rc);
		dc.ExcludeClipRect(&rc);
	}

	GetClientRect(&rc);
	rc.bottom -= 3;
	if (!m_bmBkgndCache.IsNull()) {
		CDC dcw; dcw.CreateCompatibleDC(0);
		HBITMAP hbm = dcw.SelectBitmap(m_bmBkgndCache);
		dc.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), dcw, 0, 0, SRCCOPY);
		dcw.SelectBitmap(hbm);
	} else {
		dc.FillSolidRect(&rc, m_clrPane);
	}
	rc.bottom += 3;

	for (int x = rc.left; x < rc.right; x += m_sizeTab.cx) {
		m_imPanel.Draw(dc, 0, x, rc.bottom - m_sizeTab.cy, ILD_TRANSPARENT);
	}
	dc.RestoreDC(save);
}

BOOL CLccTabExCtrl::OnEraseBkgnd(HDC hdc)
{
	return TRUE;
}

void CLccTabExCtrl::OnSize(UINT nType, int cx, int cy)
{
	if (nType != SIZE_MINIMIZED) {
		CalcItemBounds();
		Invalidate(TRUE);
	}
}

UINT CLccTabExCtrl::OnNcCalcSize(BOOL bCalcValidateRect, NCCALCSIZE_PARAMS* pncp)
{
	return (bCalcValidateRect ? 0 : WVR_ALIGNTOP);
}

UINT CLccTabExCtrl::OnNcHitTest(int xPos, int yPos)
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

void CLccTabExCtrl::OnStyleChanged(WPARAM, LPARAM)
{
	CalcItemBounds();
	Invalidate(TRUE);
}

void CLccTabExCtrl::OnPrintClient(HDC hdc, LPARAM flags)
{
	if (flags & PRF_ERASEBKGND)
		DefWindowProc(WM_ERASEBKGND, (WPARAM)hdc, 0);

	if (flags & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED))
		OnDraw(hdc);
}

void CLccTabExCtrl::OnPaint()
{
	CPaintDC dc(m_hWnd);
	OnDraw(dc.m_hDC);
}

void CLccTabExCtrl::OnDraw(CDCHandle dc)
{
	int numItems = (int)m_array.GetCount();
	if (m_bDoNotRedraw || numItems < 1) return;

	CRect rc;
	GetClientRect(&rc);
	CDC dcw; dcw.CreateCompatibleDC(dc);
	HBITMAP hbmT = dcw.SelectBitmap(::CreateCompatibleBitmap(dc, rc.Width(), rc.Height()));
	EraseBkgnd(dcw, false);

	// 最後のタブから描く
	CRect rcs;
	for (int i = numItems - 1; i >= 0; i--) {
		if (i == m_selIndex) continue;

		if (GetItemRect(i, &rcs) && dc.RectVisible(&rcs))
			DrawItem(dcw, rcs, i);
	}
	if (GetItemRect(m_selIndex, &rcs) && dc.RectVisible(&rcs))
		DrawItem(dcw, rcs, m_selIndex);

//	// 最初のタブから最後のタブまでの区間をBitBltする
//	GetTabAreaRect(&rc);
//	// 最終タブのその後ろ15pixel分はアニメーションのために描いておく
//	m_imPanel.Draw(dcw, 0, rc.right, rc.top, ILD_TRANSPARENT);
//	dc.BitBlt(rc.left, rc.top, rc.Width() + m_sizeTab.cx, rc.Height(), dcw, rc.left, rc.top, SRCCOPY);
	dc.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), dcw, rc.left, rc.top, SRCCOPY);

	// 追加ボタン
	GetAddButtonRect(&rc);
	if (dc.RectVisible(&rc)) {
		m_imAdd.Draw(dcw, m_bAddButtonPressed ? 2 : (m_bInAddButton ? 1 : 0), rc.left, rc.top, ILD_TRANSPARENT);
		dc.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), dcw, rc.left, rc.top, SRCCOPY);
	}

	::DeleteObject(dcw.SelectBitmap(hbmT));
	// If we haven't set the current focus yet, set it now.
	// Only happens when we first paint the tab controls
	if (m_focusIndex == -1)
		SetCurFocus(m_selIndex);
}

void CLccTabExCtrl::DrawItem(CDC& dc, const CRect& rcItem, int index)
{
	// Get the rectangle for the item.
	CRect rc = rcItem;

	// 何も描くスペースが無い？
	if (rc.Width() < m_sizeTab.cx * 2)
		return;

	CTabItem& item = m_array[index];
	LONG style  = GetStyle();
	bool bSelected = (index == m_selIndex);
	bool bHot = (index == m_hotIndex);
	bool bHilite = ((item.m_state & TCIS_HIGHLIGHTED) != 0);
	bool bFocus = (!(style & TCS_FOCUSNEVER) && (index == m_focusIndex));

	int offset = (bSelected ? 1 : (bHilite ? 10 : (bHot ? 4 : (bFocus ? 10 : 7))));

	// 左右の片
	m_imPanel.Draw(dc, offset, rc.left, rc.bottom - m_sizeTab.cy, ILD_TRANSPARENT);
	m_imPanel.Draw(dc, offset + 2, rc.right - m_sizeTab.cx, rc.bottom - m_sizeTab.cy, ILD_TRANSPARENT);

	CRect rcInner = rc;
	rcInner.InflateRect(-m_sizeTab.cx, 0);

	// 何か描くスペースがある？
	if (rcInner.left < rcInner.right) {
		// タブ矩形内部の描画
		// 内部背景
		for (int x = rcInner.left; x < rcInner.right; x += m_sizeTab.cx) {
			int dx = min(m_sizeTab.cx, rcInner.right - x);
			m_imPanel.DrawEx(offset + 1, dc, x, rc.top, dx, m_sizeTab.cy, CLR_NONE, CLR_NONE, ILD_TRANSPARENT);
		}

		// アイコン
		if (!m_imIcon.IsNull() && (item.m_iImage >= 0)) {
			if (rcInner.Width() < m_sizeIcon.cx)
				return;

			m_imIcon.Draw(dc, item.m_iImage, rcInner.left, rcInner.top + (rcInner.Height() - m_sizeIcon.cy) / 2, ILD_TRANSPARENT);
			rcInner.left += m_sizeIcon.cx + EXTRA_ICON_PADDING;
		}

		// ロック済み or クローズボタン
		CRect rcw;
		if (item.isLocked()) {
			rcInner.right -= (m_sizeClose.cx + EXTRA_CLOSE_PADDING);
			if (rcInner.left >= rcInner.right)
				return;

			int iImage = (bSelected ? 1 : 0);
			m_imLock.Draw(dc, iImage, rcInner.right + EXTRA_CLOSE_PADDING, rcInner.top + (rcInner.Height() - m_sizeClose.cy) / 2, ILD_TRANSPARENT);
		} else if (m_exStyle & TCS_EX_CLOSEBUTTON) {
			rcInner.right -= (m_sizeClose.cx + EXTRA_CLOSE_PADDING);
			if (rcInner.left >= rcInner.right)
				return;

			bool bHotClose = ((m_hotIndex == index) && (m_hotIndexHitTest & TCHT_ONITEMCLOSE));
//@			int iImage = m_bCloseButtonPressed ? 3 : (bHotClose ? 2 : (bSelected ? 1 : 0));
			int iImage = m_bCloseButtonPressed ? 2 : (bHotClose ? 1 : 0);
			m_imClose.Draw(dc, iImage, rcInner.right + EXTRA_CLOSE_PADDING, rcInner.top + (rcInner.Height() - m_sizeClose.cy) / 2, ILD_TRANSPARENT);
		}

		// テキスト
		if (!item.m_strText.IsEmpty()) {
			if (m_bUseGdipDrawing) {
				Graphics g(dc);
				Font* pFont = GetFont(bSelected);
				GdipDrawString(&g, pFont, item.m_strText, &rcInner, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS, bHilite ? RGB(192, 0, 0) : item.m_clrText);
			} else {
				HFONT hf = GetFontHandle(bSelected);
				DrawString(dc, hf, item.m_strText, &rcInner, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS, bHilite ? RGB(192, 0, 0) : item.m_clrText);
			}
		}
	}
}

DWORD CLccTabExCtrl::SetExtendedStyle(DWORD mask, DWORD exStyle)
{
	DWORD old = m_exStyle;
	m_exStyle = ((m_exStyle & ~mask) | exStyle);

	if ((old ^ m_exStyle) & TCS_EX_CLOSEBUTTON) {
		CalcItemBounds();
		Invalidate(TRUE);
	}
	return old;
}

Font* CLccTabExCtrl::GetFont(bool bBold)
{
	NONCLIENTMETRICS ncm = { sizeof(ncm) };
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, FALSE);

	CClientDC dc(NULL);
	if (bBold) {
		if (!m_pFontBold) {
			ncm.lfMenuFont.lfWeight = FW_BOLD;
			m_pFontBold = new Font(dc, &ncm.lfMenuFont);
		}
		return m_pFontBold;
	} else {
		if (!m_pFont) {
			m_pFontBold = new Font(dc, &ncm.lfMenuFont);
		}
		return m_pFont;
	}
}

HFONT CLccTabExCtrl::GetFontHandle(bool bBold)
{
	if (m_font.IsNull() || m_fontBold.IsNull()) {
		NONCLIENTMETRICS ncm = { sizeof(ncm) };
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, FALSE);
		m_font.CreateFontIndirect(&ncm.lfMenuFont);
		ncm.lfMenuFont.lfWeight = FW_BOLD;
		m_fontBold.CreateFontIndirect(&ncm.lfMenuFont);
	}
	return bBold ? m_fontBold.m_hFont : m_font.m_hFont;
}

HIMAGELIST CLccTabExCtrl::SetImageList(HIMAGELIST himNew)
{
	HIMAGELIST himPrev = m_imIcon.Detach();
	m_imIcon.Attach(himNew);
	if (!m_imIcon.IsNull())
		m_imIcon.GetIconSize(m_sizeIcon);

	CalcItemBounds();
	InvalidateTabArea();
	return himPrev;
}

COLORREF CLccTabExCtrl::SetPaneColor(COLORREF clr)
{
	COLORREF clrTemp = m_clrPane;
	if (m_clrPane != clr) {
		m_clrPane = clr;
		Invalidate(TRUE);
	}
	return clrTemp;
}

void CLccTabExCtrl::RecalcTabHeight()
{
//	m_tabHeight = m_sizeTab.cy;
	m_tabHeight = 40;
}

void CLccTabExCtrl::SendSelChangeNotify(int indexOld, int indexNew)
{
	NMTCSELCHANGE sc;
	sc.hdr.hwndFrom = m_hWnd;
	sc.hdr.idFrom = GetDlgCtrlID();
	sc.hdr.code = TCN_SELCHANGE;
	sc.iItemOld = indexOld;
	sc.iItemNew = indexNew;
	m_owner.SendMessage(WM_NOTIFY, (WPARAM)sc.hdr.idFrom, (LPARAM)&sc);
}

BOOL CLccTabExCtrl::SendSimpleNotify(UINT code, int index)
{
	NMTAB tab;
	memset(&tab, 0, sizeof(tab));
	tab.hdr.hwndFrom = m_hWnd;
	tab.hdr.idFrom = GetDlgCtrlID();
	tab.hdr.code = code;
	tab.iItem = index;
	if (index >= 0 && index < (int)m_array.GetCount()) {
		CTabItem& item = m_array[index];
		tab.lParam = item.m_param;
	}
	return (BOOL)m_owner.SendMessage(WM_NOTIFY, (WPARAM)tab.hdr.idFrom, (LPARAM)&tab);
}

int CLccTabExCtrl::GetCurFocus()
{
	if (m_focusIndex < 0) {
		// FIXME("we have not to return negative value");
		return 0;
	}
	return m_focusIndex;
}

BOOL CLccTabExCtrl::GetItemRect(int index, RECT* prc)
{
	// Perform a sanity check and a trivial visibility check.
	if (isBadIndex(index)) {
		// need to initialize these to empty rects
		if (prc) {
			SetRectEmpty(prc);
			prc->bottom = m_tabHeight;
		}
		return FALSE;
	}

	CTabItem& item = m_array[index];
	CRect rcItem = item.m_rect;

	CRect rc;
	GetClientRect(&rc);

	// valign = bottom で計算
	rcItem.bottom = rc.bottom;
	rcItem.top = rcItem.bottom - m_sizeTab.cy;

	// Check for visibility
	if (prc)
		*prc = rcItem;
	return (rcItem.left < rc.right) && (rcItem.right > rc.left);
}

void CLccTabExCtrl::GetTabAreaRect(RECT* prc)
{
	if (prc) SetRectEmpty(prc);

	int numItems = (int)m_array.GetCount();
	if (numItems > 0) {
		CRect rcs, rct;
		GetItemRect(0, &rcs);
		GetItemRect(numItems - 1, &rct);

		if (prc) {
			prc->left = rcs.left;
			prc->top = rcs.top;
			prc->right = rct.right;
			prc->bottom = rct.bottom;
		}
	}
}

void CLccTabExCtrl::InvalidateTabArea()
{
	if (IsWindowVisible()) {
		CRect rc;
		GetTabAreaRect(&rc);
		rc.right += m_sizeTab.cx;
		if (!(m_exStyle & TCS_EX_CLOSEBUTTON_RA))
			rc.right += m_sizeAdd.cx;
		InvalidateRect(&rc, TRUE);
	}
}

void CLccTabExCtrl::GetAddButtonRect(RECT* prc)
{
	if (!prc) return;

	if (m_exStyle & TCS_EX_CLOSEBUTTON_RA) {
		GetClientRect(prc);
		prc->right -= (CXSPACER + m_cxBarPaddingRight);
		prc->left = prc->right - m_sizeAdd.cx;
		prc->top += ((prc->bottom - prc->top) - m_sizeAdd.cy) / 2 + 1;
		prc->bottom = prc->top + m_sizeAdd.cy;
	} else {
		CRect rc;
		GetTabAreaRect(&rc);
		prc->left = rc.right;
		prc->right = prc->left + m_sizeAdd.cx;
		prc->top += (rc.Height() - m_sizeAdd.cy) / 2 + 1;
		prc->bottom = prc->top + m_sizeAdd.cy;
	}
}

int CLccTabExCtrl::SetCurSel(int index)
{
	int prevIndex = -1;
	if (!isBadIndex(index)) {
		prevIndex = m_selIndex;
		m_selIndex = index;
		InvalidateTabArea();
		UpdateWindow();
	}
	return prevIndex;
}

void CLccTabExCtrl::SetCurFocus(int index)
{
	if (isBadIndex(index)) return;

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

int CLccTabExCtrl::InternalHitTest(POINT pt, UINT* flags)
{
	// タブの検査する順番はタブの重なり具合を考慮
	// 1.現在選択中, 2.一番最初のタブ, 3.2番目, ...
	CSimpleArray<int> arr;
	arr.Add(m_selIndex);
	int i;
	int numItems = (int)m_array.GetCount();
	for (i = 0; i <= numItems; i++) {
		if (i != m_selIndex)
			arr.Add(i);
	}
	CRect rc;
	for (i = 0; i < arr.GetSize(); i++) {
		int index = arr[i];
		GetItemRect(index, &rc);
		if (rc.PtInRect(pt)) {
			rc.InflateRect(-m_sizeTab.cx, 0);

			if (flags) {
				CRect r;
				// まずアイコン位置
				if (!m_imIcon.IsNull()) {
					r = rc;
					r.right = rc.left + m_sizeIcon.cx + EXTRA_ICON_PADDING;
					if (r.PtInRect(pt)) {
						*flags = TCHT_ONITEMICON;
						return index;
					}
				}

				// 次はクローズボタン
				if (m_exStyle & TCS_EX_CLOSEBUTTON) {
					r = rc;
					r.left = r.right - m_sizeClose.cx;
					r.top += (r.Height() - m_sizeClose.cy) / 2;
					r.bottom = r.top + m_sizeClose.cy;
					if (r.PtInRect(pt)) {
						*flags = TCHT_ONITEMCLOSE;
						return index;
					}
				}
				*flags = TCHT_ONITEMLABEL;
			}
			return index;
		}
	}

	if (flags) {
		// 追加ボタン
		GetAddButtonRect(&rc);
		if (rc.PtInRect(pt))
			*flags = TCHT_ONADDBUTTON;
		else
			*flags = TCHT_NOWHERE;
	}
	return -1;
}

int CLccTabExCtrl::InternalHitTestForDrag(POINT pt)
{
	CRect rcc;
	GetClientRect(&rcc);
	if (!rcc.PtInRect(pt))
		return -1;

	int i;
	CRect rc;
	int numItems = (int)m_array.GetCount();
	for (i = 0; i < numItems; i++) {
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

void CLccTabExCtrl::RecalcHotTrack(const POINT* ppt, int* pIndexLeave, int* pIndexEnter)
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
	} else if (hittest != m_hotIndexHitTest) {
		m_hotIndexHitTest = hittest;
		if (pIndexEnter) *pIndexEnter = m_hotIndex;
	}

	m_bInAddButton = (hittest == TCHT_ONADDBUTTON);
}

int CLccTabExCtrl::GetMinTabWidth()
{
	return (m_tabMinWidth < 0) ? (DEFAULT_MIN_TAB_WIDTH + (!m_imIcon.IsNull() ? m_sizeIcon.cx : 0)) : m_tabMinWidth;
}

void CLccTabExCtrl::CalcItemBounds()
{
	if (!m_bHeightSet)
		RecalcTabHeight();

	CClientDC dc(m_hWnd);
	Graphics g(dc);
	// ボールドフォントで計測
	Font* pFont = GetFont(true);
	StringFormat fmt;
	fmt.SetFormatFlags(StringFormatFlagsNoWrap);

	int tabMinWidth = GetMinTabWidth();

	int cxTab, posLeftSide = m_cxBarPaddingLeft;
	int i;
	int numItems = (int)m_array.GetCount();
	for (i = 0; i < numItems; i++) {
		CTabItem& item = m_array[i];

		// Set the leftmost position of the tab.
		item.m_rect.left = posLeftSide;

		cxTab = m_sizeTab.cx * 2;
		if (item.m_strText.IsEmpty()) {
			cxTab += tabMinWidth;
		} else {
			// Calculate how wide the tab is depending on the text it contains
			RectF r(0, 0, 0, 0);
			PointF origin(0.0f, 0.0f);
			g.MeasureString(item.m_strText, item.m_strText.GetLength(), pFont, origin, &fmt, &r);
			cxTab += (int)ceil(r.Width);
			cxTab = max(cxTab, m_tabMinWidth);
		}

		// イメージリストアイコンがある場合
		if (m_sizeIcon.cx > 0)
			cxTab += m_sizeIcon.cx + EXTRA_ICON_PADDING;

		// ロック済み
		if (item.isLocked())
			cxTab += m_sizeClose.cx + EXTRA_CLOSE_PADDING;
		// クローズボタンがある場合
		else if (m_exStyle & TCS_EX_CLOSEBUTTON)
			cxTab += m_sizeClose.cx + EXTRA_CLOSE_PADDING;

		item.m_rect.right = item.m_rect.left + cxTab;
		item.m_rect.bottom = 0;
		item.m_rect.top = 0;

		// The leftmost position of the next item is the rightmost position of this one.
		posLeftSide = item.m_rect.right + m_cxTabSpacing;
	}

	if (numItems > 0) {
		posLeftSide -= m_cxTabSpacing;		// 余分に足した分を戻す

		CRect rc;
		GetClientRect(&rc);
		rc.left += m_cxBarPaddingLeft;
		rc.right -= (m_sizeAdd.cx + CXSPACER + m_cxBarPaddingRight);

		if (posLeftSide >= rc.right) {
			// クライアント矩形の幅より、タブ全体の幅が大きい場合
			// 比率を計算して、各タブを縮小する
			double scale = (double)rc.Width() / (double)(posLeftSide - m_cxBarPaddingLeft);

			posLeftSide = m_cxBarPaddingLeft;
			for (i = 0; i < numItems; i++) {
				CTabItem& item = m_array[i];

				cxTab = item.m_rect.Width();

				// Set the leftmost position of the tab.
				item.m_rect.left = posLeftSide;
				cxTab = FLOOR_INT(cxTab * scale);
				cxTab = max(cxTab, tabMinWidth);

				item.m_rect.right = item.m_rect.left + cxTab;
				posLeftSide = item.m_rect.right + m_cxTabSpacing;
			}
		}
	}

	RecalcHotTrack(NULL, NULL, NULL);
}

void CLccTabExCtrl::OnSetFocus(WPARAM, LPARAM)
{
	if (!(GetStyle() & TCS_FOCUSNEVER))
		FocusChanging();
	else
		GetParent().SetFocus();
}

void CLccTabExCtrl::OnKillFocus(WPARAM, LPARAM)
{
	if (m_bTrackMouseEvent) {
		m_bTrackMouseEvent = FALSE;

		TRACKMOUSEEVENT te;
		te.cbSize = sizeof(te);
		te.dwFlags = TME_HOVER | TME_LEAVE | TME_CANCEL;
		te.hwndTrack = m_hWnd;
		te.dwHoverTime = HOVER_DEFAULT;
		_TrackMouseEvent(&te);
	}

	if (!(GetStyle() & TCS_FOCUSNEVER))
		FocusChanging();
}

void CLccTabExCtrl::FocusChanging()
{
	CRect rc;
	if (GetItemRect(m_focusIndex, &rc))
		InvalidateRect(&rc, FALSE);
}

void CLccTabExCtrl::AdjustRect(BOOL fLarger, LPRECT prc)
{
	int iWholeHeight = m_tabHeight;
	if (fLarger) {	// Go from display rectangle
		// Add the height of the tabs.
		prc->top -= iWholeHeight;
	}
    else {			// Go from window rectangle.
		prc->top += iWholeHeight;
	}
}

int CLccTabExCtrl::SetMinTabWidth(int cx)
{
	int oldcx = m_tabMinWidth;
	m_tabMinWidth = cx;
	CalcItemBounds();
	return oldcx;
}

void CLccTabExCtrl::GetBarMetrics(NMTCMETRICS* ptcm)
{
	if (ptcm) {
		ptcm->cxBarPaddingLeft = m_cxBarPaddingLeft;
		ptcm->cxBarPaddingRight = m_cxBarPaddingRight;
	}
}

void CLccTabExCtrl::SetBarMetrics(NMTCMETRICS* ptcm)
{
	if (ptcm) {
		m_cxBarPaddingLeft = ptcm->cxBarPaddingLeft;
		m_cxBarPaddingRight = ptcm->cxBarPaddingRight;

		// タブ幅の再計算
		CalcItemBounds();
		Invalidate(TRUE);
	}
}

int CLccTabExCtrl::InsertItem(int index, const TCITEM* tcItem)
{
	if (index < 0) return -1;

	int numItems = (int)m_array.GetCount();
	if (index > numItems)
		index = numItems;

	CTabItem itemNew;
	if (tcItem->mask & TCIF_TEXT)	itemNew.m_strText = tcItem->pszText;
	if (tcItem->mask & TCIF_IMAGE)	itemNew.m_iImage = tcItem->iImage;
	if (tcItem->mask & TCIF_PARAM)	itemNew.m_param = tcItem->lParam;

	if (numItems == 0)
		m_selIndex = 0;

	m_array.InsertAt(index, itemNew);
	numItems = (int)m_array.GetCount();

	// タブ幅の再計算
	CalcItemBounds();

	// アニメーションでタブを拡張しながら表示する
	// 対象のタブ幅を保存してから 0 にする
	CTabItem& item = m_array[index];

	// 描画を限定する矩形
	CRect rc;
	GetClientRect(&rc);
	rc.left = item.m_rect.left;
	rc.right = m_array[numItems - 1].m_rect.right + m_sizeTab.cx;

	int width = item.m_rect.Width() - DEFAULT_MIN_TAB_WIDTH;
	// 以後のタブ情報を更新
	int i;
	for (i = index; i < numItems; i++)
		m_array[i].m_rect.OffsetRect(-width, 0);

	const int delta = 20;
	while (width > 0) {
		int cx = min(delta, width);
		for (i = index; i < numItems; i++)
			m_array[i].m_rect.OffsetRect(cx, 0);

		RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		width -= cx;
	}
	return index;
}

BOOL CLccTabExCtrl::DeleteItem(int index)
{
	if (isBadIndex(index)) return FALSE;

	// まずアニメーションして削除対象のタブを処理
	int numItems = (int)m_array.GetCount();
	CTabItem& item = m_array[index];
	int width = item.m_rect.Width() - DEFAULT_MIN_TAB_WIDTH;

	const int delta = 20;
	CRect rc;
	GetClientRect(&rc);
	rc.left = item.m_rect.left;
	rc.right = m_array[numItems - 1].m_rect.right + delta;
	while (width > 0) {
		int cx = min(delta, width);
		for (int i = index; i < numItems; i++)
			m_array[i].m_rect.OffsetRect(-cx, 0);

		RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		rc.right -= cx;
		width -= cx;
	}

	// 親に削除を通知
	int selSave = m_selIndex;				// TCN_DELETEITEMを送った先でm_selIndexが変更されることもある
	SendSimpleNotify(TCN_DELETEITEM, index);

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
		if (numItems == 0)
			m_selIndex = -1;
	}

	m_array.RemoveAt(index);
	--numItems;

	// Reposition and repaint tabs
	CalcItemBounds();

	InvalidateTabArea();
	return TRUE;
}

BOOL CLccTabExCtrl::DeleteAllItems()
{
	int numItems = (int)m_array.GetCount();
	while (numItems) {
		SendSimpleNotify(TCN_DELETEITEM, 0);
		m_array.RemoveAt(0);
		numItems--;
	}

	m_selIndex = -1;
	m_focusIndex = -1;
	m_hotIndex = -1;
	m_hotIndexHitTest = TCHT_NOWHERE;

	CalcItemBounds();
	Invalidate(TRUE);

	return TRUE;
}

BOOL CLccTabExCtrl::SetItemT(int index, TCITEM* tcItem)
{
	if (isBadIndex(index)) return FALSE;

	bool recalc = false;
	CTabItem& item = m_array[index];
	if (tcItem->mask & TCIF_IMAGE)
		item.m_iImage = tcItem->iImage;
	if (tcItem->mask & TCIF_PARAM)
		item.m_param = tcItem->lParam;
	if (tcItem->mask & TCIF_STATE)
		item.m_state = tcItem->dwState;
	if (tcItem->mask & TCIF_TEXT) {
		CString strOld = item.m_strText.GetString();
		item.m_strText = tcItem->pszText;
		// Update and repaint tabs
		if (strOld.Compare(item.m_strText)) {
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

BOOL CLccTabExCtrl::GetItemT(int index, TCITEM* tcItem)
{
	if (isBadIndex(index)) return FALSE;

	CTabItem& item = m_array[index];
	if (tcItem->mask & TCIF_IMAGE)	tcItem->iImage = item.m_iImage;
	if (tcItem->mask & TCIF_PARAM)	tcItem->lParam = item.m_param;
	if (tcItem->mask & TCIF_STATE)	tcItem->dwState = item.m_state;
	if (tcItem->mask & TCIF_TEXT)	lstrcpyn(tcItem->pszText, item.m_strText, tcItem->cchTextMax);

	return TRUE;
}

DWORD CLccTabExCtrl::GetItemState(int index)
{
	if (isBadIndex(index)) return (DWORD)-1;

	return m_array[index].m_state;
}

DWORD CLccTabExCtrl::SetItemState(int index, DWORD state, DWORD stateMask)
{
	if (isBadIndex(index)) return (DWORD)-1;

	CTabItem& item = m_array[index];
	DWORD newState = (item.m_state & ~stateMask) | state;

	if (item.m_state != newState) {
		item.m_state = newState;
		CRect rc;
		if (GetItemRect(index, &rc))
			InvalidateRect(&rc, FALSE);
	}
	return newState;
}

BOOL CLccTabExCtrl::isStateItem(int index, DWORD checkState)
{
	DWORD state = GetItemState(index);
	if (state != (DWORD)-1)
		return ((state & checkState) != 0);
	return FALSE;
}

HBITMAP CLccTabExCtrl::CreateDragImage(int index, POINT* ppt)
{
	HBITMAP hbm = NULL;
	int numItems = (int)m_array.GetCount();
	if (0 <= index && index < numItems) {
		CClientDC dc(NULL);
		CDC dcw;
		dcw.CreateCompatibleDC(dc);

		CRect rc;
		GetItemRect(index, &rc);
		rc.OffsetRect(-rc.left, -rc.top);

		hbm = CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
		if (hbm) {
			HBITMAP hbmT = dcw.SelectBitmap(hbm);
			// 下地
			dcw.FillRect(&rc, GetSysColorBrush(COLOR_WINDOW));
			DrawItem(dcw, rc, index);
			dcw.SelectBitmap(hbmT);
		}

		if (ppt) {
			// カーソル位置（ビットマップ上のオフセット位置）
			GetItemRect(index, &rc);
			ppt->x -= rc.left;
			ppt->y -= rc.top;
		}
	}
	return hbm;
}

LRESULT CLccTabExCtrl::OnTcmSetBackgroundImage(WPARAM wParam, LPARAM lParam)
{
	if (!m_bmBkgndCache.IsNull())
		m_bmBkgndCache.DeleteObject();

	HBITMAP hbmT = m_bmBkgnd.Detach();
	m_bmBkgnd.Attach((HBITMAP)lParam);
	if (!m_bmBkgnd.IsNull()) {
		BITMAP bm;
		m_bmBkgnd.GetBitmap(bm);

		CClientDC dc(NULL);
		int width = GetSystemMetrics(SM_CXSCREEN);
		int height = m_tabHeight;

		m_bmBkgndCache.CreateCompatibleBitmap(dc, width, height);
		CDC dcs; dcs.CreateCompatibleDC(dc);
		HBITMAP hbms = dcs.SelectBitmap(m_bmBkgnd);
		CDC dcd; dcd.CreateCompatibleDC(dc);
		HBITMAP hbmd = dcd.SelectBitmap(m_bmBkgndCache);

		dcd.SetStretchBltMode(HALFTONE);
		dcd.SetBrushOrg(0, 0);
		dcd.StretchBlt(0, 0, width, height, dcs, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
		dcd.SelectBitmap(hbmd);
		dcs.SelectBitmap(hbms);
	}
	Invalidate(TRUE);
	return (LRESULT)hbmT;
}

////////////////////////////////////////////////////////////////////////
//

extern "C" LRESULT CALLBACK TabExWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CLccTabExCtrl* pWnd = new CLccTabExCtrl;
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}

