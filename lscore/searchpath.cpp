#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"
#include "gdiplus_sup.h"
#include "tbb_customdraw.h"

////////////////////////////////////////////////////////////////////////
//

#define EDITPADDING				2

#define KEYDATA_ALT             0x2000
#define KEYDATA_PREVSTATE       0x4000

#define SBN_ALTDOWN				0
#define SBN_DOWN				1
#define SBN_UP					2


class CSPEdit : public CWindowImpl<CSPEdit, CEdit>
{
private:
	bool m_bReadOnly;

public:
	CSPEdit()
		: m_bReadOnly(false)
	{
	}

	void SetReadOnly(bool bReadOnly)
	{
		m_bReadOnly = bReadOnly;
	}

BEGIN_MSG_MAP(CSPEdit)
	ON_MESSAGE(WM_GETDLGCODE, OnGetDlgCode)
	MESSAGE_HANDLER(WM_SYSKEYDOWN, OnHandlerSysKeyDown)
	MESSAGE_HANDLER(WM_KEYDOWN, OnHandlerKeyDown)
	MESSAGE_HANDLER(WM_CHAR, OnHandlerChar)
	MESSAGE_HANDLER(WM_MOUSEWHEEL, OnHandlerMouseWheel)
END_MSG_MAP()

	LRESULT OnGetDlgCode(WPARAM wParam, LPARAM lParam)
	{
		LRESULT code = DLGC_HASSETSEL | DLGC_WANTCHARS | DLGC_WANTARROWS;
		if (lParam && (((MSG*)lParam)->message == WM_KEYDOWN)) {
			int vkey = (int)((MSG*)lParam)->wParam;
#ifdef SUPPORT_RETURN_KEY_PROCESSING
			if (vkey == VK_RETURN)
				code |= DLGC_WANTMESSAGE;
#endif
			if (vkey == VK_TAB) {
				if (GetKeyState(VK_SHIFT) < 0) {
					// shiftキーが押下されていれば何もしない
					;
				}
				else {
					// shiftキーが押下されていなければ自前で処理 → ボタンへフォーカス移動
					CWindow wnd = ::GetNextWindow(m_hWnd, GW_HWNDNEXT);
					if (wnd.IsWindow()) {
						code |= DLGC_WANTTAB;
						wnd.SetFocus();
					}
				}
			}
		}
		return code;
	}

	LRESULT OnHandlerSysKeyDown(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (KEYDATA_ALT & HIWORD(lParam)) {
			if (wParam == VK_DOWN) {
				bool bHasFocus = (GetFocus() == m_hWnd);
				if (bHasFocus) HideCaret();
				GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), SBN_ALTDOWN), (LPARAM)m_hWnd);
				if (bHasFocus) ShowCaret();
				return 0;
			}
		}
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnHandlerKeyDown(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
#ifdef SUPPORT_RETURN_KEY_PROCESSING
		if (wParam == VK_RETURN) {
			return 0;
		}
#endif

		if (wParam == VK_DOWN) {
			GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), SBN_DOWN), (LPARAM)m_hWnd);
			return 0;
		}
		else if (wParam == VK_UP) {
			GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), SBN_UP), (LPARAM)m_hWnd);
			return 0;
		}

		if (m_bReadOnly) {
			switch (wParam) {
			case VK_DELETE:
			case VK_BACK:
				return 0;
			}
		}
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnHandlerChar(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_bReadOnly) {
			return 0;
		}
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnHandlerMouseWheel(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		UINT nFlags = GET_KEYSTATE_WPARAM(wParam);
		short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

		CRect rc;
		GetWindowRect(&rc);
		if (!rc.PtInRect(pt)) {
			bHandled = FALSE;
			return 1;
		}

		int updown = (zDelta > 0) ? SBN_UP : SBN_DOWN;
		GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), updown), (LPARAM)m_hWnd);
		return 0;
	}
};


////////////////////////////////////////////////////////////////////////
//

class CSEItemArray : public CSimpleArray<CString>
{
public:
	CSimpleArray<int> m_arrImageIndex;

	BOOL Add(int iImage, LPCTSTR pszText)
	{
		m_arrImageIndex.Add(iImage);
		return CSimpleArray<CString>::Add(pszText);
	}

	BOOL Delete(int index)
	{
		if (index < 0 || index >= GetSize())
			return FALSE;

		m_arrImageIndex.RemoveAt(index);
		return RemoveAt(index);
	}

	void DeleteAll()
	{
		m_arrImageIndex.RemoveAll();
		RemoveAll();
	}

	int GetImageIndex(int index)
	{
		if (index < 0 || index >= GetSize())
			return -1;
		return m_arrImageIndex[index];
	}

	BOOL SetImageIndex(int index, int iImage)
	{
		return m_arrImageIndex.SetAtIndex(index, iImage);
	}
};

////////////////////////////////////////////////////////////////////////
// CSearchPathCtrl

#define IDC_EDIT				100
#define IDC_BAR					101
#define IDC_LISTBOX				102

#define ID_MENUCOMMAND_START	0x6000
#define ID_MENUCOMMAND_END		0x60ff

class CSearchPathCtrl
			: public CWindowImpl<CSearchPathCtrl>
			, public CTBarCustomDrawImpl<CSearchPathCtrl>
{
private:
	CSPEdit m_edit;
	CToolBarCtrl m_bar;
	HIMAGELIST m_himTBar;
	CSize m_sizeTBar;
	HFONT m_hFont;
	CSEItemArray m_list;
	HIMAGELIST m_hImage;
	CSize m_sizeImage;
	TEXTMETRIC m_TextMetric;
	CFont m_fontMarlett;
	CRect m_rcDrop;
	int m_indexSel;
	bool m_bTrackMouseEvent:1;
	bool m_bPressed:1;
	bool m_bHot:1;
	bool m_bEditChangeSkipOnce:1;
	gdiplus_using m_gu;

public:
	CSearchPathCtrl()
		: m_himTBar(NULL)
		, m_hFont(NULL)
		, m_hImage(NULL)
		, m_indexSel(-1)
		, m_bTrackMouseEvent(false)
		, m_bPressed(false)
		, m_bHot(false)
		, m_bEditChangeSkipOnce(false)
	{
		m_sizeImage = CSize(0, 0);
		memset(&m_TextMetric, 0, sizeof(m_TextMetric));
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		if (m_himTBar)
			ImageList_Destroy(m_himTBar);

		delete this;
	}

BEGIN_MSG_MAP(CSearchPathCtrl)
	CHAIN_MSG_MAP(CTBarCustomDrawImpl<CSearchPathCtrl>)
	ON_WM_GETDLGCODE()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_GETFONT()
	ON_WM_SETFONT()
	MESSAGE_HANDLER(WM_ENABLE, OnHandlerEnable)
	MESSAGE_HANDLER(WM_NCPAINT, OnHandlerNcPaint)
	ON_WM_ERASEBKGND()
	ON_WM_PRINTCLIENT()
	ON_WM_PAINT()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(WM_MENUSELECT, OnMenuSelect)
	MESSAGE_HANDLER(WM_MENUCHAR, OnMenuChar)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(WM_GETTEXT, OnGetText)
	ON_MESSAGE(WM_GETTEXTLENGTH, OnGetTextLength)
	ON_BN_CLICKED(IDC_BAR, OnBrowseFolder)
	COMMAND_ID_HANDLER(IDC_EDIT, OnEditCommandHandler)
	ON_MESSAGE(EM_GETSEL, OnEditGetSel)
	ON_MESSAGE(EM_SETSEL, OnEditSetSel)
	ON_MESSAGE(EM_SETLIMITTEXT, OnEditSetLimitText)
	ON_MESSAGE(EM_GETLIMITTEXT, OnEditGetLimitText)
	ON_MESSAGE(EM_SETCUEBANNER, OnEditSetCueBanner)
	ON_MESSAGE(EM_GETCUEBANNER, OnEditGetCueBanner)
	ON_MESSAGE(SEM_ADDSTRING, OnListAddString)
	ON_MESSAGE(SEM_DELETESTRING, OnListDeleteString)
	ON_MESSAGE(SEM_RESETCONTENT, OnListResetContent)
	ON_MESSAGE(SEM_SETCURSEL, OnListSetCurSel)
	ON_MESSAGE(SEM_GETCURSEL, OnListGetCurSel)
	ON_MESSAGE(SEM_GETCOUNT, OnListGetCount)
	ON_MESSAGE(SEM_SELECTSTRING, OnListSelectString)
	ON_MESSAGE(SEM_FINDSTRING, OnListFindString)
	ON_MESSAGE(SEM_LIMITTEXT, OnSetLimitText)
	ON_MESSAGE(SEM_SHOWDROPDOWN, OnListShowDropDown)
	ON_MESSAGE(SEM_GETEDITSEL, OnGetEditSel)
	ON_MESSAGE(SEM_SETEDITSEL, OnSetEditSel)
	ON_MESSAGE(SEM_SETREADONLY, OnSetReadOnly)
	ON_MESSAGE(SEM_GETIMAGELIST, OnGetImageList)
	ON_MESSAGE(SEM_SETIMAGELIST, OnSetImageList)
	ON_MESSAGE(SEM_GETITEMIMAGE, OnGetItemImage)
	ON_MESSAGE(SEM_SETITEMIMAGE, OnSetItemImage)
END_MSG_MAP()

	LRESULT OnGetDlgCode()
	{
		return 0;		// m_edit.SendMessage(WM_GETDLGCODE, 0, 0);
	}

	int OnCreate(LPCREATESTRUCT pcs)
	{
		// エディットコントロール
		DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | ES_LEFT | ES_AUTOHSCROLL;
		m_edit.Create(m_hWnd, NULL, NULL, style, 0, IDC_EDIT, NULL);
		if (pcs->style & SES_READONLY)
			m_edit.SetReadOnly(true);

		// ブラウズボタン（ツールバー）
		style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
				CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NOMOVEY |
				TBSTYLE_FLAT | TBSTYLE_TRANSPARENT;
		m_bar.Create(m_hWnd, NULL, NULL, style, 0, IDC_BAR);

		m_bar.SetButtonStructSize();
		m_himTBar = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_PICKER_FOLDER), 14, RGB(255, 0, 255));
		if (m_himTBar) {
			int cx, cy;
			ImageList_GetIconSize(m_himTBar, &cx, &cy);
			m_sizeTBar = CSize(cx, cy);
			m_bar.SetImageList(m_himTBar);
		}

		TBBUTTON tbbtn = { 0 };
		tbbtn.idCommand = IDC_BAR;
		tbbtn.fsState = TBSTATE_ENABLED;
		tbbtn.fsStyle = TBSTYLE_BUTTON;
		m_bar.AddButtons(1, &tbbtn);
		m_bar.SetBitmapSize(m_sizeTBar.cx, m_sizeTBar.cy);

		// ツールバーをクライアント矩形内に収める
		CRect rc;
		GetClientRect(&rc);
		const int cxyButtonMargin = 7;
		m_bar.SetButtonSize(m_sizeTBar.cx + cxyButtonMargin, rc.Height());
		AddCustomDrawBar(m_bar);

		// オートコンプリート
		SHAutoComplete(m_edit, SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYS_DIRS);
		return 1;
	}

	void OnSize(UINT flags, int cx, int cy)
	{
		if (GetStyle() & SES_DROPDOWN)
			m_rcDrop.SetRect(cx - GetSystemMetrics(SM_CXVSCROLL), 0, cx, cy);
		else
			m_rcDrop.SetRectEmpty();

		CRect rc;
		m_bar.GetItemRect(0, &rc);
		m_edit.MoveWindow(EDITPADDING, 0, cx - (rc.Width() + m_rcDrop.Width() + EDITPADDING * 2), cy, TRUE);
		m_bar.MoveWindow(cx - (rc.Width() + m_rcDrop.Width()), 0, rc.Width(), cy, TRUE);
	}

	bool DrawClientEdge(HRGN hRgnUpdate = NULL, HBRUSH hBrush = NULL)
	{
		CWindowDC dc(m_hWnd);
		if (dc.IsNull())
			return false;

		// Get border size
		int cxBorder = GetSystemMetrics(SM_CXBORDER);
		int cyBorder = GetSystemMetrics(SM_CYBORDER);

		CRect rc;
		GetWindowRect(&rc);

		// Remove the client edge from the update region
		int cxEdge = GetSystemMetrics(SM_CXEDGE);
		int cyEdge = GetSystemMetrics(SM_CYEDGE);
		rc.InflateRect(-cxEdge, -cyEdge);
		CRgn rgn;
		rgn.CreateRectRgnIndirect(&rc);
		if (rgn.IsNull())
			return false;

		if (hRgnUpdate != NULL)
			rgn.CombineRgn(hRgnUpdate, rgn, RGN_AND);

		rc.OffsetRect(-rc.left, -rc.top);

		rc.OffsetRect(cxEdge, cyEdge);
		dc.ExcludeClipRect(&rc);
		rc.InflateRect(cxEdge, cyEdge);

		// 外枠を塗る
		COLORREF clr = mix_color(GetSysColor(COLOR_3DFACE), GetSysColor(COLOR_HIGHLIGHT), 50);
		dc.FillSolidRect(&rc, clr);

		// Use background brush too, since theme border might not cover everything
		if (cxBorder < cxEdge && cyBorder < cyEdge) {
			if (hBrush == NULL)
				// need conditional code because types don't match in winuser.h
#ifdef _WIN64
				hBrush = (HBRUSH)::GetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND);
#else
				hBrush = (HBRUSH)UlongToPtr(::GetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND));
#endif

			rc.InflateRect(cxBorder - cxEdge, cyBorder - cyEdge);
			dc.FillRect(&rc, hBrush);
		}
		DefWindowProc(WM_NCPAINT, (WPARAM)rgn.m_hRgn, 0L);
		return true;
	}

	LRESULT OnHandlerEnable(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lr = DefWindowProc();
		// Edit
		m_edit.SendMessage(WM_ENABLE, wParam, lParam);
		// Toolbar
		int cnt = m_bar.GetButtonCount();
		for (int i = 0; i < cnt; i++) {
			TBBUTTON tbb;
			m_bar.GetButton(i, &tbb);
			m_bar.EnableButton(tbb.idCommand, (BOOL)wParam);
		}
		Invalidate();
		return lr;
	}

	LRESULT OnHandlerNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		HRGN hRgn = (wParam != 1) ? (HRGN)wParam : NULL;
		if (DrawClientEdge(hRgn))
			bHandled = TRUE;
		return 0;
	}

	BOOL OnEraseBkgnd(HDC hdc)
	{
		CRect rc;
		GetClientRect(&rc);
		CDCHandle dc = hdc;
		dc.FillRect(&rc, IsWindowEnabled() ? COLOR_WINDOW : COLOR_BTNFACE);
		return TRUE;
	}

	void OnPrintClient(HDC hdc, LPARAM flags)
	{
		if (flags & PRF_ERASEBKGND)
			SendMessage(WM_ERASEBKGND, (WPARAM)hdc, 0);

		if (flags & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED))
			OnDraw(hdc);
	}

	void OnPaint()
	{
		CPaintDC dc(m_hWnd);
		OnDraw(dc);
	}

	void GradationRect(CDCHandle dc, const CRect& rc, COLORREF clrBorder, COLORREF clrLighten, COLORREF clrDarken)
	{
		// グラデーションの作成
		// Draw the individual band backgrounds
		COLORREF clrBkgnd = GetSysColor(COLOR_3DFACE);
		COLORREF clrBandF = mix_color(clrBkgnd, clrLighten, 30);
		COLORREF clrBandT = mix_color(clrBkgnd, clrDarken, 50);

		CRect rcc = rc;
		rcc.InflateRect(-1, -1);
		CRect rcw = rcc;
		rcw.bottom = rcc.Height() / 3;
		dc.FillSolidRect(&rcw, clrBandF);
		rcw.top = rcw.bottom;
		rcw.bottom = rcc.bottom;
		FillShadeRect(dc, rcw, clrBandF, clrBandT, true);

		if (clrBorder != CLR_NONE) {
			dc.FillSolidRect(rc.left, rc.top, 1, rc.Height(), clrBorder);
			dc.FillSolidRect(rc.left, rc.top, rc.Width(), 1, clrBorder);
			dc.FillSolidRect(rc.right - 1, rc.top, 1, rc.Height(), clrBorder);
			dc.FillSolidRect(rc.left, rc.bottom - 1, rc.Width(), 1, clrBorder);
		}
	}

	void DrawDropdownButton(HDC hdc)
	{
		if (!(GetStyle() & SES_DROPDOWN)) return;

		CDCHandle dc = hdc;
		bool bDisabled  = (m_list.GetSize() < 1);
		if (!bDisabled) {
			if (m_bPressed) {
				GradationRect(dc, m_rcDrop, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DFACE), GetSysColor(COLOR_3DSHADOW));
			}
			else if (m_bHot) {
				draw_rect(dc, &m_rcDrop, mix_color(GetSysColor(COLOR_3DFACE), GetSysColor(COLOR_HIGHLIGHT), 50), GetSysColor(COLOR_WINDOW));
			}
			else
				dc.FillRect(&m_rcDrop, COLOR_WINDOW);
		}

		static const int nFontHeight = 13;
		if (m_fontMarlett.IsNull())
			m_fontMarlett.CreateFont(nFontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, SYMBOL_CHARSET, 0, 0, 0, 0, _T("Marlett"));

		TCHAR ch = _T('6');
		HFONT hfont = dc.SelectFont(m_fontMarlett);
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(GetSysColor(bDisabled ? COLOR_GRAYTEXT : COLOR_WINDOWTEXT));
		dc.DrawText(&ch, 1, &m_rcDrop, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP);
		dc.SelectFont(hfont);
	}

	void OnDraw(HDC hdc)
	{
		DrawDropdownButton(hdc);
	}

	void OnSetFocus(HWND hOldWnd)
	{
		m_edit.SetFocus();
		if (!(GetStyle() & SES_READONLY))
			m_edit.SetSel(0, -1);
	}

	void OnSetFont(HFONT hFont, BOOL bRedraw)
	{
		m_hFont = hFont;
		m_edit.SetFont(m_hFont);
		m_bar.SetFont(m_hFont);

		CClientDC dc(nullptr);
		HFONT htmp = dc.SelectFont(m_hFont);
		dc.GetTextMetrics(&m_TextMetric);
		dc.SelectFont(htmp);

		CRect rce;
		m_edit.GetWindowRect(&rce);

		ScreenToClient(&rce);
		rce.bottom = rce.top + m_TextMetric.tmHeight + m_TextMetric.tmExternalLeading + 1;

		CRect rc;
		GetClientRect(&rc);
		m_edit.MoveWindow(rc.left + EDITPADDING, rc.top + (rc.Height() - rce.Height()) / 2, rce.Width(), rce.Height(), TRUE);

		if (bRedraw)
			Invalidate(TRUE);
	}

	void OnLButtonDown(UINT nFlags, CPoint pt)
	{
		// フォーカスを取得する
		if (GetFocus() != m_edit)
			m_edit.SetFocus();

		if ((GetStyle() & SES_DROPDOWN) && m_rcDrop.PtInRect(pt))
			OnListDropDown();
	}

	void OnMouseMove(UINT nFlags, CPoint point)
	{
		if (!m_bTrackMouseEvent) {
			TRACKMOUSEEVENT te;
			te.cbSize = sizeof(te);
			te.dwFlags = TME_LEAVE;
			te.hwndTrack = m_hWnd;
			te.dwHoverTime = HOVER_DEFAULT;
			m_bTrackMouseEvent = (_TrackMouseEvent(&te) != FALSE);
		}

		if (m_bTrackMouseEvent) {
			if ((GetStyle() & SES_DROPDOWN) && (m_list.GetSize() > 0)) {
				bool bHot = (m_rcDrop.PtInRect(point) != FALSE);
				if (bHot != m_bHot) {
					m_bHot = bHot;
					InvalidateRect(&m_rcDrop, FALSE);
				}
			}
		}
	}

	LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam)
	{
		m_bTrackMouseEvent = false;
		m_bHot = false;

		if (GetStyle() & SES_DROPDOWN)
			InvalidateRect(&m_rcDrop, FALSE);
		return 0;
	}

	void SetEditTextFromList(int index)
	{
		if (0 <= index && index < m_list.GetSize()) {
			m_bEditChangeSkipOnce = true;
			m_edit.SetWindowText(m_list[index]);
			m_edit.SetSel(0, -1);
		}
	}

	LRESULT OnMenuSelect(WPARAM wParam, LPARAM lParam)
	{
		UINT item = (UINT)LOWORD(wParam);			// メニュー項目またはポップアップメニューのインデックス
		UINT flags = (UINT)HIWORD(wParam);			// メニュー フラグ
//@		HMENU hmenu = (HMENU)lParam;
		if (flags != 0xffff) {
			if (!(flags & (MF_DISABLED | MF_GRAYED | MF_POPUP | MF_SYSMENU | MF_MOUSESELECT))) {
				m_indexSel = (int)(item - ID_MENUCOMMAND_START);
				SetEditTextFromList(m_indexSel);
			}
		}
		return 0;
	}

	LRESULT OnMenuChar(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		TCHAR ch = (TCHAR)LOWORD(wParam);			// ASCII文字
		UINT flag = (UINT)HIWORD(wParam);			// メニュー フラグ
		HMENU hmenu = (HMENU)lParam;
		if (!(flag & MF_SYSMENU)) {
			if (ch == VK_TAB)
				return MAKELONG(0, 1);
		}
		bHandled = FALSE;
		return 1;
	}

	inline HFONT OnGetFont()
	{
		return m_hFont;
	}

	inline LRESULT OnSetText(WPARAM wParam, LPARAM lParam)
	{
		return m_edit.SendMessage(WM_SETTEXT, wParam, lParam);
	}

	inline LRESULT OnGetText(WPARAM wParam, LPARAM lParam)
	{
		return m_edit.SendMessage(WM_GETTEXT, wParam, lParam);
	}

	inline LRESULT OnGetTextLength(WPARAM wParam, LPARAM lParam)
	{
		return m_edit.SendMessage(WM_GETTEXTLENGTH, wParam, lParam);
	}

	inline LRESULT OnEditGetSel(WPARAM wParam, LPARAM lParam)
	{
		return m_edit.SendMessage(EM_GETSEL, wParam, lParam);
	}

	inline LRESULT OnEditSetSel(WPARAM wParam, LPARAM lParam)
	{
		return m_edit.SendMessage(EM_SETSEL, wParam, lParam);
	}

	inline LRESULT OnSetReadOnly(WPARAM wParam, LPARAM lParam)
	{
		m_edit.SetReadOnly((wParam != 0));
		return TRUE;
	}

	inline LRESULT OnEditSetLimitText(WPARAM wParam, LPARAM lParam)
	{
		return m_edit.SendMessage(EM_SETLIMITTEXT, wParam, lParam);
	}

	inline LRESULT OnEditGetLimitText(WPARAM wParam, LPARAM lParam)
	{
		return m_edit.SendMessage(EM_GETLIMITTEXT, wParam, lParam);
	}

	inline LRESULT OnEditSetCueBanner(WPARAM wParam, LPARAM lParam)
	{
		return m_edit.SendMessage(EM_SETCUEBANNER, wParam, lParam);
	}

	inline LRESULT OnEditGetCueBanner(WPARAM wParam, LPARAM lParam)
	{
		return m_edit.SendMessage(EM_GETCUEBANNER, wParam, lParam);
	}

	void OnBrowseFolder()
	{
		if (GetFocus() != m_edit)
			m_edit.SetFocus();

		TCHAR buf[MAX_PATH * 2];
		m_edit.GetWindowText(buf, ARRAYSIZE(buf));

		NMSEGETTEXT sgt;
		memset(&sgt, 0, sizeof(sgt));
		sgt.hdr.hwndFrom = m_hWnd;
		sgt.hdr.idFrom = ::GetDlgCtrlID(m_hWnd);
		sgt.hdr.code = SEN_GETTEXT;
		sgt.pszText = buf;
		sgt.cchTextMax = ARRAYSIZE(buf);

		if (GetParent().SendMessage(WM_NOTIFY, (WPARAM)sgt.hdr.idFrom, (LPARAM)&sgt)) {
			m_edit.SetWindowText(buf);
		}
	}

	void OnMeasureItem(int idCtrl, LPMEASUREITEMSTRUCT pmi)
	{
		if (pmi->CtlType != ODT_MENU) return;

		CRect rc;
		GetWindowRect(&rc);
		pmi->itemWidth = rc.Width() - GetSystemMetrics(SM_CXMENUCHECK) - GetSystemMetrics(SM_CXEDGE) * 2 - GetSystemMetrics(SM_CXBORDER) * 2;
		pmi->itemHeight = max(m_TextMetric.tmHeight + m_TextMetric.tmExternalLeading, m_sizeImage.cy) + 2;
	}

	void OnDrawItem(int idCtrl, LPDRAWITEMSTRUCT pdi)
	{
		if (pdi->CtlType != ODT_MENU) return;

		CDCHandle dc = pdi->hDC;
		CRect rc = pdi->rcItem;

		dc.FillSolidRect(&rc, GetSysColor((pdi->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_MENU));

		rc.left += 2;
		if (m_hImage) {
			int iImage = m_list.GetImageIndex((int)pdi->itemData);
			ImageList_Draw(m_hImage, iImage, dc, rc.left, rc.top + (rc.Height() - m_sizeImage.cy) / 2, ILD_TRANSPARENT);
			rc.left += m_sizeImage.cx + 4;
		}
		CString str = m_list[(int)pdi->itemData];
		if (!str.IsEmpty()) {
			HFONT hfont = dc.SelectFont(m_hFont);

			int bkm = dc.SetBkMode(TRANSPARENT);
			COLORREF clr = GetSysColor((pdi->itemState & ODS_GRAYED) ? COLOR_GRAYTEXT :
											((pdi->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT));
			clr = dc.SetTextColor(clr);
			dc.DrawText(str, -1, &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
			dc.SetTextColor(clr);
			dc.SetBkMode(bkm);

			dc.SelectFont(hfont);
		}
	}

	void TrackListMenu()
	{
		int cnt = m_list.GetSize();
		if (cnt >= 0) {
			// ボタン押下
			m_bPressed = true;
			InvalidateRect(&m_rcDrop, FALSE);

			// キャレット非表示
			GUITHREADINFO info;
			memset(&info, 0, sizeof(info));
			info.cbSize = sizeof(info);
			GetGUIThreadInfo(::GetWindowThreadProcessId(m_hWnd, NULL), &info);
			if ((info.flags & GUI_CARETBLINKING) && (info.hwndCaret == m_edit))
				::HideCaret(info.hwndCaret);

			// ポップアップメニュー作成
			CMenu pop;
			pop.CreatePopupMenu();

			// 高さ制限
			MENUINFO mi;
			mi.cbSize = sizeof(mi);
			mi.fMask = MIM_MAXHEIGHT;
			mi.cyMax = (m_TextMetric.tmHeight + m_TextMetric.tmExternalLeading) * 16 + GetSystemMetrics(SM_CYEDGE) * 2;
			pop.SetMenuInfo(&mi);

			// 履歴をリストアップ
			for (int i = 0; i < cnt; i++)
				pop.AppendMenu(MF_OWNERDRAW, ID_MENUCOMMAND_START + i, (LPCTSTR)(INT_PTR)i);

			CRect rc;
			GetWindowRect(&rc);
			int cmd = pop.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, rc.left, rc.bottom, m_hWnd);

			// 選択文字列を編集ウィンドウへセット
			int index = cmd - ID_MENUCOMMAND_START;
			if (index >= 0 && index < m_list.GetSize()) {
				m_indexSel = index;
				SetEditTextFromList(m_indexSel);
			}

			// キャレット表示
			if ((info.flags & GUI_CARETBLINKING) && (info.hwndCaret == m_edit))
				::ShowCaret(info.hwndCaret);

			// ボタン戻す
			m_bPressed = false;
			InvalidateRect(&m_rcDrop, FALSE);
		}
	}

	BOOL OnEditCommandHandler(UINT code, UINT id, HWND hwndCtrl, BOOL& bHandled)
	{
		NMHDR hdr;
		switch (code) {
//		case EN_SETFOCUS: break;
//		case EN_KILLFOCUS: break;
//		case EN_ERRSPACE: break;
//		case EN_MAXTEXT: break;
//		case EN_HSCROLL: break;
//		case EN_VSCROLL: break;
		case SBN_ALTDOWN: 		OnListDropDown(); break;
		case SBN_DOWN:			OnListDown(); break;
		case SBN_UP:			OnListUp(); break;
		case EN_CHANGE:
			OnEditChanged();
			hdr.code = SEN_EDITCHANGE;
			hdr.hwndFrom = m_hWnd;
			hdr.idFrom = GetDlgCtrlID();
			GetParent().SendMessage(WM_NOTIFY, (WPARAM)hdr.idFrom, (LPARAM)&hdr);
			break;
		case EN_UPDATE:
			hdr.code = SEN_EDITUPDATE;
			hdr.hwndFrom = m_hWnd;
			hdr.idFrom = GetDlgCtrlID();
			GetParent().SendMessage(WM_NOTIFY, (WPARAM)hdr.idFrom, (LPARAM)&hdr);
			break;
		}
		return TRUE;
	}

	void OnEditChanged()
	{
		if (m_bEditChangeSkipOnce) {
			m_bEditChangeSkipOnce = false;
			return;
		}

		CString str;
		if (m_edit.GetWindowText(str) < 1)
			m_indexSel = -1;
		else
			m_indexSel = FindString(-1, str, FALSE, FALSE);
	}

	void OnListDown()
	{
		int numItems = m_list.GetSize();
		if (numItems < 1) return;

		int selNew = m_indexSel;
		if (selNew < 0) {
			// まだ未選択
			selNew = 0;
		}
		else {
			if (m_indexSel < numItems - 1) {
				CString str;
				m_edit.GetWindowText(str);
				selNew = FindString(m_indexSel + 1, str, FALSE, FALSE);
				if (selNew < 0)
					selNew = m_indexSel + 1;
			}
		}

		if (m_indexSel != selNew) {
			m_indexSel = selNew;
			m_bEditChangeSkipOnce = true;
			m_edit.SetWindowText(m_list[m_indexSel]);
			m_edit.SetSel(0, -1);
		}
	}

	void OnListUp()
	{
		int numItems = m_list.GetSize();
		if (numItems < 1) return;

		int selNew = m_indexSel;
		if (selNew < 0) {
			// まだ未選択
			selNew = 0;
		}
		else {
			if (m_indexSel > 0) {
				CString str;
				m_edit.GetWindowText(str);
				selNew = ReverseFindString(m_indexSel - 1, str, FALSE, FALSE);
				if (selNew < 0)
					selNew = m_indexSel - 1;
			}
		}

		if (m_indexSel != selNew) {
			m_indexSel = selNew;
			m_bEditChangeSkipOnce = true;
			m_edit.SetWindowText(m_list[m_indexSel]);
			m_edit.SetSel(0, -1);
		}
	}

	void OnListDropDown()
	{
		if (!(GetStyle() & SES_DROPDOWN))
			return;
		if (m_bPressed)
			return;
		int numItems = m_list.GetSize();
		if (numItems < 1)
			return;

		if (m_list.GetSize() > 0) {
			TrackListMenu();
		}
	}

	LRESULT OnListAddString(WPARAM wParam, LPARAM lParam)
	{
		m_list.Add((int)wParam, (LPCTSTR)lParam);
		return (LRESULT)m_list.GetSize();
	}

	LRESULT OnListDeleteString(WPARAM wParam, LPARAM lParam)
	{
		int index = (int)wParam;
		if (index < 0 || index >= m_list.GetSize())
			return SEM_ERR;

		m_list.RemoveAt(index);

		return (LRESULT)SEM_OK;
	}

	LRESULT OnListResetContent(WPARAM wParam, LPARAM lParam)
	{
		m_list.DeleteAll();
		m_edit.SetWindowText(NULL);
		m_indexSel = SEM_ERR;

		return (LRESULT)SEM_OK;
	}

	LRESULT OnListSetCurSel(WPARAM wParam, LPARAM lParam)
	{
		int index = (int)wParam;
		if (index < 0 || index >= m_list.GetSize()) {
			m_edit.SetWindowText(NULL);
			return SEM_ERR;
		}

		m_indexSel = index;
		m_bEditChangeSkipOnce = true;
		m_edit.SetWindowText(m_list[index]);
		m_edit.SetSel(0, -1);

		return (LRESULT)SEM_OK;
	}

	LRESULT OnListGetCurSel(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)m_indexSel;
	}

	LRESULT OnListGetCount(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)m_list.GetSize();
	}

	int FindString(int indexStart, LPCTSTR pszFind, BOOL bExact, BOOL bWrapArround)
	{
		int numItems = m_list.GetSize();
		if (numItems < 1)
			return SEM_ERR;

		if (indexStart < 0)
			indexStart = 0;
		if (indexStart >= numItems)
			return SEM_ERR;

		if (!pszFind || *pszFind == 0)
			return SEM_ERR;

		int len = lstrlen(pszFind);
		int iFound = SEM_ERR;
		int i;
		for (i = indexStart; i < numItems; i++) {
			if (bExact) {
				if (!StrCmpI(m_list[i], pszFind)) {
					iFound = i;
					break;
				}
			}
			else {
				if (!StrCmpNI(m_list[i], pszFind, len)) {
					iFound = i;
					break;
				}
			}
		}
		if (iFound < 0 && bWrapArround) {
			for (i = 0; i < indexStart; i++) {
				if (bExact) {
					if (!StrCmpI(m_list[i], pszFind)) {
						iFound = i;
						break;
					}
				}
				else {
					if (!StrCmpNI(m_list[i], pszFind, len)) {
						iFound = i;
						break;
					}
				}
			}
		}
		return iFound;
	}

	int ReverseFindString(int indexStart, LPCTSTR pszFind, BOOL bExact, BOOL bWrapArround)
	{
		int numItems = m_list.GetSize();
		if (numItems < 1)
			return SEM_ERR;

		if (indexStart < 0)
			indexStart = numItems - 1;
		if (indexStart >= numItems)
			return SEM_ERR;

		if (!pszFind || *pszFind == 0)
			return SEM_ERR;

		int len = lstrlen(pszFind);
		int iFound = SEM_ERR;
		int i;
		for (i = indexStart; i >= 0; i--) {
			if (bExact) {
				if (!StrCmpI(m_list[i], pszFind)) {
					iFound = i;
					break;
				}
			}
			else {
				if (!StrCmpNI(m_list[i], pszFind, len)) {
					iFound = i;
					break;
				}
			}
		}
		if (iFound < 0 && bWrapArround) {
			for (i = numItems - 1; i > indexStart; i--) {
				if (bExact) {
					if (!StrCmpI(m_list[i], pszFind)) {
						iFound = i;
						break;
					}
				}
				else {
					if (!StrCmpNI(m_list[i], pszFind, len)) {
						iFound = i;
						break;
					}
				}
			}
		}
		return iFound;
	}

	inline LRESULT OnListFindString(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)FindString((int)wParam, (LPCTSTR)lParam, FALSE, TRUE);
	}

	inline LRESULT OnListFindStringExact(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)FindString((int)wParam, (LPCTSTR)lParam, TRUE, TRUE);
	}

	int SelectString(int indexStart, LPCTSTR pszFind, BOOL bExact, BOOL bWrapArround)
	{
		int iFound = FindString(indexStart, pszFind, bExact, bWrapArround);
		if (iFound >= 0) {
			m_indexSel = iFound;
			m_bEditChangeSkipOnce = true;
			m_edit.SetWindowText(m_list[iFound]);
			m_edit.SetSel(0, -1);
			return SEM_OK;
		}
		return SEM_ERR;
	}

	inline LRESULT OnListSelectString(WPARAM wParam, LPARAM lParam)
	{
		return SelectString((int)wParam, (LPCTSTR)lParam, FALSE, TRUE);
	}

	inline LRESULT OnListSelectStringExact(WPARAM wParam, LPARAM lParam)
	{
		return SelectString((int)wParam, (LPCTSTR)lParam, TRUE, TRUE);
	}

	inline LRESULT OnSetLimitText(WPARAM wParam, LPARAM lParam)
	{
		return m_edit.SendMessage(EM_SETLIMITTEXT, wParam, lParam);
	}

	LRESULT OnListShowDropDown(WPARAM wParam, LPARAM lParam)
	{
		if ((GetStyle() & SES_DROPDOWN) && wParam)
			OnListDropDown();
		return (LRESULT)SEM_OK;
	}

	inline LRESULT OnGetEditSel(WPARAM wParam, LPARAM lParam)
	{
		return m_edit.SendMessage(EM_GETSEL, wParam, lParam);
	}

	inline LRESULT OnSetEditSel(WPARAM wParam, LPARAM lParam)
	{
		return m_edit.SendMessage(EM_SETSEL, wParam, lParam);
	}

	inline LRESULT OnGetImageList(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)m_hImage;
	}

	LRESULT OnSetImageList(WPARAM wParam, LPARAM lParam)
	{
		HIMAGELIST htmp = m_hImage;
		m_hImage = (HIMAGELIST)lParam;

		if (!m_hImage)
			m_sizeImage = CSize(0, 0);
		else {
			int cx, cy;
			ImageList_GetIconSize(m_hImage, &cx, &cy);
			m_sizeImage = CSize(cx, cy);
		}
		return (LRESULT)htmp;
	}

	inline LRESULT OnGetItemImage(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)m_list.GetImageIndex((int)wParam);
	}

	inline LRESULT OnSetItemImage(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)m_list.SetImageIndex((int)wParam, (int)lParam);
	}
};


////////////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK SearchPathWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CSearchPathCtrl* pWnd = new CSearchPathCtrl();
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}
