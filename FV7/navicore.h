////////////////////////////////////////////////////////////////////////
// navicore.h

#pragma once
#ifndef __NAVICORE_H__
#define __NAVICORE_H__

#include "resource.h"

#define KEYDATA_ALT             0x2000
#define KEYDATA_PREVSTATE       0x4000
#define SBN_ALTDOWN				0
#define SBN_DOWN				1
#define SBN_UP					2

////////////////////////////////////////////////////////////////////////
//

class CSPEdit : public CWindowImpl<CSPEdit, CEdit>
{
private:
	bool m_bReadOnly;
	bool m_bFocus;

public:
	CSPEdit()
		: m_bReadOnly(false), m_bFocus(false)
	{
	}

	void SetReadOnly(bool bReadOnly)
	{
		m_bReadOnly = bReadOnly;
	}

	bool HasFocus()
	{
		return m_bFocus;
	}

BEGIN_MSG_MAP(CSPEdit)
	ON_MESSAGE(WM_GETDLGCODE, OnGetDlgCode)
	MESSAGE_HANDLER(WM_SETFOCUS, OnHandlerSetFocus)
	MESSAGE_HANDLER(WM_KILLFOCUS, OnHandlerKillFocus)
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
				} else {
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

	LRESULT OnHandlerSetFocus(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_bFocus = true;
		PostMessage(EM_SETSEL, 0, (LPARAM)-1);

		NMHDR nh;
		nh.idFrom = GetDlgCtrlID();
		nh.hwndFrom = m_hWnd;
		nh.code = NM_SETFOCUS;
		GetParent().SendMessage(WM_NOTIFY, (WPARAM)nh.idFrom, (LPARAM)&nh);

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnHandlerKillFocus(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_bFocus = false;

		NMHDR nh;
		nh.idFrom = GetDlgCtrlID();
		nh.hwndFrom = m_hWnd;
		nh.code = NM_KILLFOCUS;
		GetParent().SendMessage(WM_NOTIFY, (WPARAM)nh.idFrom, (LPARAM)&nh);

		bHandled = FALSE;
		return 1;
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
		} else if (wParam == VK_UP) {
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
	CSimpleArray<LPARAM> m_arrParam;

	BOOL Add(int iImage, LPCTSTR pszText, LPARAM param)
	{
		m_arrImageIndex.Add(iImage);
		m_arrParam.Add(param);
		return CSimpleArray<CString>::Add(pszText);
	}

	BOOL Delete(int index)
	{
		if (index < 0 || index >= GetSize())
			return FALSE;

		m_arrImageIndex.RemoveAt(index);
		m_arrParam.RemoveAt(index);
		return RemoveAt(index);
	}

	void DeleteAll()
	{
		m_arrImageIndex.RemoveAll();
		m_arrParam.RemoveAll();
		RemoveAll();
	}

	int GetImageIndex(int index)
	{
		if (index < 0 || index >= GetSize())
			return -1;
		return m_arrImageIndex[index];
	}

	bool SetImageIndex(int index, int iImage)
	{
		if (index < 0 || index >= GetSize())
			return false;
		m_arrImageIndex[index] = iImage;
		return true;
	}

	LPARAM GetParam(int index)
	{
		if (index < 0 || index >= GetSize())
			return 0;
		return m_arrParam[index];
	}
};

////////////////////////////////////////////////////////////////////////
// CNaviCoreCtrl

#define EDITPADDING				2

#define IDC_EDIT_INNER			8100
#define IDC_BAR_INNER			8101
#define ID_MENUCOMMAND_START	0x6000
#define ID_MENUCOMMAND_END		0x60ff

#define NVCN_GETDISPINFO		1
#define NVCN_BEGINDRAG			2
#define NVCN_ENDDRAG			3
#
#define NVCIF_TEXT              0x00000001
#define NVCIF_IMAGE             0x00000002
#define NVCIF_LPARAM            0x00000020
#define NVCIF_DI_SETITEM        0x10000000

typedef struct tagNAVICOREITEM {
	UINT mask;
	INT_PTR iItem;
	LPCWSTR pszText;
	int cchTextMax;
	int iImage;
	LPARAM lParam;
} NAVICOREITEM;

typedef struct {
	NMHDR hdr;
	NAVICOREITEM item;
} NMNAVICORE;


class CNaviCoreCtrl
			: public CWindowImpl<CNaviCoreCtrl>
			, public CTBarCustomDrawImpl<CNaviCoreCtrl>
{
private:
	CSPEdit m_edit;
#ifdef GOTOFOLDER_BUTTON
	CToolBarCtrl m_bar;
	HIMAGELIST m_himTBar;
#endif
	HIMAGELIST m_hImage;
	CSize m_sizeTBar;
	CSize m_sizeImage;
	HFONT m_hFont;
	CSEItemArray m_list;
	TEXTMETRIC m_TextMetric;
	CFont m_fontMarlett;
	CRect m_rTDrop;
	int m_indexSel;
	bool m_bTrackMouseEvent:1;
	bool m_bPressed:1;
	bool m_bHot:1;
	bool m_bEditChangeSkipOnce:1;
	gdiplus_using m_gu;

public:
	CNaviCoreCtrl()
		: m_hFont(NULL)
#ifdef GOTOFOLDER_BUTTON
		, m_himTBar(NULL)
#endif
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

BEGIN_MSG_MAP(CNaviCoreCtrl)
	CHAIN_MSG_MAP(CTBarCustomDrawImpl<CNaviCoreCtrl>)
	ON_WM_GETDLGCODE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_GETFONT()
	ON_WM_SETFONT()
	MESSAGE_HANDLER(WM_SETCURSOR, OnHandlerSetCursor)
	MESSAGE_HANDLER(WM_NCPAINT, OnNcPaint)
	ON_WM_PRINTCLIENT()
	ON_WM_PAINT()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_NOTIFY_CODE(TTN_GETDISPINFO, OnToolTipText)
	ON_NOTIFY_CODE(NM_SETFOCUS, OnNotifySetFocus)
	ON_NOTIFY_CODE(NM_KILLFOCUS, OnNotifyKillFocus)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(WM_MENUSELECT, OnMenuSelect)
	MESSAGE_HANDLER(WM_MENUCHAR, OnMenuChar)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(WM_GETTEXT, OnGetText)
	ON_MESSAGE(WM_GETTEXTLENGTH, OnGetTextLength)
#ifdef GOTOFOLDER_BUTTON
	ON_COMMAND(IDM_DIR_GOTO, OnCommandGoTo)
#endif
	COMMAND_ID_HANDLER(IDC_EDIT_INNER, OnEditCommandHandler)
END_MSG_MAP()

	LRESULT OnGetDlgCode()
	{
		return 0;		// m_edit.SendMessage(WM_GETDLGCODE, 0, 0);
	}

	int OnCreate(LPCREATESTRUCT pcs)
	{
		// エディットコントロール
		DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | ES_LEFT | ES_AUTOHSCROLL;
		m_edit.Create(m_hWnd, NULL, NULL, style, 0, IDC_EDIT_INNER, NULL);

#ifdef GOTOFOLDER_BUTTON
		// ブラウズボタン（ツールバー）
		style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
				CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NOMOVEY |
				TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT;
		m_bar.Create(m_hWnd, NULL, NULL, style, 0, IDC_BAR_INNER);

		m_bar.SetButtonStructSize();
		m_himTBar = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_NAVIINNER), 12, RGB(255, 0, 255));
		if (m_himTBar) {
			int cx, cy;
			ImageList_GetIconSize(m_himTBar, &cx, &cy);
			m_sizeTBar = CSize(cx, cy);
			m_bar.SetImageList(m_himTBar);
		}

		TBBUTTON tbbtn = { 0 };
		tbbtn.iBitmap = 1;
		tbbtn.idCommand = IDM_DIR_GOTO;
		tbbtn.fsState = TBSTATE_ENABLED;
		tbbtn.fsStyle = TBSTYLE_BUTTON;
		m_bar.AddButtons(1, &tbbtn);
		m_bar.SetBitmapSize(m_sizeTBar.cx, m_sizeTBar.cy);

		// ツールバーをクライアント矩形内に収める
		CRect rc;
		GetClientRect(&rc);
		const int cxyBtnMargin = 7;
		m_bar.SetButtonSize(m_sizeTBar.cx + cxyBtnMargin, rc.Height());
		AddCustomDrawBar(m_bar);
#endif

		// オートコンプリート
		SHAutoComplete(m_edit, SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYS_DIRS);
		return 1;
	}

	void OnDestroy()
	{
#ifdef GOTOFOLDER_BUTTON
		if (m_himTBar)
			ImageList_Destroy(m_himTBar);
#endif
	}

	void OnSize(UINT flags, int cx, int cy)
	{
		m_rTDrop.SetRect(cx - GetSystemMetrics(SM_CXVSCROLL), 0, cx, cy);
#ifdef GOTOFOLDER_BUTTON
		CRect rct(0, 0, 0, 0);
		int numButtons = m_bar.GetButtonCount();
		if (numButtons > 0) {
			CRect rcs, rce;
			m_bar.GetItemRect(0, &rcs);
			m_bar.GetItemRect(numButtons - 1, &rce);
			rct.UnionRect(&rcs, &rce);
		}
#endif

		CRect rc(0, 0, cx, cy);
		rc.right -= m_rTDrop.Width();					// ドロップダウンボタン分を引く
#ifdef GOTOFOLDER_BUTTON
		m_bar.MoveWindow(rc.right - rct.Width(), 0, rct.Width(), cy, TRUE);
		rc.right -= rct.Width();						// ツールバーボタン分を引く
#endif
		rc.left += EDITPADDING * 2 + m_sizeImage.cx;	// ヘッダアイコン分を足す

		CRect rce;
		m_edit.GetWindowRect(&rce);
		m_edit.MoveWindow(rc.left, (cy - rce.Height()) / 2, rc.Width(), rce.Height(), TRUE);
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
		COLORREF clr = m_edit.HasFocus() ? mix_color(GetSysColor(COLOR_3DFACE), GetSysColor(COLOR_HIGHLIGHT), 50) : GetSysColor(COLOR_3DFACE);
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

	LRESULT OnHandlerSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// アイコン上では手カーソルを表示する
		if (m_hImage) {
			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);

			CRect rc;
			GetClientRect(&rc);
			rc.left += EDITPADDING;
			rc.right = rc.left + m_sizeImage.cx;

			if (rc.PtInRect(pt)) {
				::SetCursor(::LoadCursor(NULL, IDC_HAND));
				return TRUE;
			}
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		HRGN hRgn = (wParam != 1) ? (HRGN)wParam : NULL;
		if (DrawClientEdge(hRgn))
			bHandled = TRUE;
		return 0;
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
		ShadeRect(dc, rcw, clrBandF, clrBandT, true);

		if (clrBorder != CLR_NONE) {
			dc.FillSolidRect(rc.left, rc.top, 1, rc.Height(), clrBorder);
			dc.FillSolidRect(rc.left, rc.top, rc.Width(), 1, clrBorder);
			dc.FillSolidRect(rc.right - 1, rc.top, 1, rc.Height(), clrBorder);
			dc.FillSolidRect(rc.left, rc.bottom - 1, rc.Width(), 1, clrBorder);
		}
	}

	void BorderRect(CDCHandle dc, const CRect& rc, COLORREF rgbBorder, COLORREF rgbInner)
	{
		if (rgbBorder != INVALID_COLOR) {
			dc.FillSolidRect(rc.left, rc.top, 1, rc.Height(), rgbBorder);
			dc.FillSolidRect(rc.left, rc.top, rc.Width(), 1, rgbBorder);
			dc.FillSolidRect(rc.right - 1, rc.top, 1, rc.Height(), rgbBorder);
			dc.FillSolidRect(rc.left, rc.bottom - 1, rc.Width(), 1, rgbBorder);
		}
		if (rgbInner != INVALID_COLOR) {
			CRect rcw = rc;
			rcw.InflateRect(-1, -1);
			HBRUSH hbr = dc.SelectBrush(CreateSolidBrush(rgbInner));
			dc.PatBlt(rcw.left, rcw.top, rcw.Width(), rcw.Height(), PATCOPY);
			DeleteObject(dc.SelectBrush(hbr));
		}
	}

	void DrawDropdownButton(HDC hdc)
	{
		CDCHandle dc = hdc;
		bool bDisabled  = (m_list.GetSize() < 1);
		if (!bDisabled) {
			if (m_bPressed) {
				GradationRect(dc, m_rTDrop, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DFACE), GetSysColor(COLOR_3DSHADOW));
			} else if (m_bHot) {
				BorderRect(dc, m_rTDrop, mix_color(GetSysColor(COLOR_3DFACE), GetSysColor(COLOR_HIGHLIGHT), 50), GetSysColor(COLOR_WINDOW));
			} else {
				dc.FillRect(&m_rTDrop, COLOR_WINDOW);
			}
		}

		static const int nFontHeight = 13;
		if (m_fontMarlett.IsNull())
			m_fontMarlett.CreateFont(nFontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, SYMBOL_CHARSET, 0, 0, 0, 0, _T("Marlett"));

		TCHAR ch = _T('6');
		HFONT hfont = dc.SelectFont(m_fontMarlett);
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(GetSysColor(bDisabled ? COLOR_GRAYTEXT : COLOR_WINDOWTEXT));
		dc.DrawText(&ch, 1, &m_rTDrop, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP);
		dc.SelectFont(hfont);
	}

	void RedrawIconHeader()
	{
		// ヘッダアイコン再描画
		if (m_hImage) {
			CRect rc;
			GetClientRect(&rc);
			rc.right = rc.left + m_sizeImage.cx + EDITPADDING * 2;
			InvalidateRect(&rc, TRUE);
		}
	}

	void OnDraw(HDC hdc)
	{
		// 左側アイコンエリア再描画
		if (m_hImage && m_indexSel >= 0) {
			CRect rc;
			GetClientRect(&rc);
			rc.right = rc.left + m_sizeImage.cx + EDITPADDING * 2;
			if (::RectVisible(hdc, &rc)) {
				CString str;
				if (GetWindowText(str) > 0 && !StrCmpI(m_list[m_indexSel], str)) {
					int iImage = GetItemImageIndex(m_indexSel);
					ImageList_Draw(m_hImage, iImage, hdc, rc.left + EDITPADDING, rc.top + (rc.Height() - m_sizeImage.cy) / 2, ILD_TRANSPARENT);
				}
			}
		}

		if (::RectVisible(hdc, &m_rTDrop))
			DrawDropdownButton(hdc);
	}

	void OnSetFocus(HWND hOldWnd)
	{
		m_edit.SetFocus();
	}

	void OnSetFont(HFONT hFont, BOOL bRedraw)
	{
		m_hFont = hFont;
		m_edit.SetFont(m_hFont);
#ifdef GOTOFOLDER_BUTTON
		m_bar.SetFont(m_hFont);
#endif

		CClientDC dc(NULL);
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

		if (m_rTDrop.PtInRect(pt))
			OnListDropDown();
		else if (m_hImage) {
			// アイコンヘッダのＤ＆Ｄ
			CRect rc;
			GetClientRect(&rc);
			rc.left += EDITPADDING;
			rc.right = rc.left + m_sizeImage.cx;
			if (rc.PtInRect(pt)) {
				CPoint pts = pt;
				ClientToScreen(&pts);
				if (lccBeginTrack(m_hWnd, pts)) {
					NMHDR hdr;
					hdr.hwndFrom = m_hWnd;
					hdr.idFrom = GetDlgCtrlID();
					hdr.code = NVCN_BEGINDRAG;
					GetParent().SendMessage(WM_NOTIFY, hdr.idFrom, (LPARAM)&hdr);
				}
			}
		}
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
			if (m_list.GetSize() > 0) {
				bool bHot = (m_rTDrop.PtInRect(point) != FALSE);
				if (bHot != m_bHot) {
					m_bHot = bHot;
					InvalidateRect(&m_rTDrop, FALSE);
				}
			}
		}
	}

	LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam)
	{
		m_bTrackMouseEvent = false;
		m_bHot = false;

		InvalidateRect(&m_rTDrop, FALSE);
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
		if (flags != 0xffff) {
			if (!(flags & (MF_DISABLED | MF_GRAYED | MF_POPUP | MF_SYSMENU | MF_MOUSESELECT))) {
				m_indexSel = (int)(item - ID_MENUCOMMAND_START);
				SetEditTextFromList(m_indexSel);
				RedrawIconHeader();
			}
		}
		return 0;
	}

	LRESULT OnMenuChar(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		TCHAR ch = (TCHAR)LOWORD(wParam);			// ASCII文字
		UINT flag = (UINT)HIWORD(wParam);			// メニュー フラグ
//@		HMENU hmenu = (HMENU)lParam;
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

	int GetItemImageIndex(int index)
	{
		int iImage = m_list.GetImageIndex(index);
		if (iImage == I_IMAGECALLBACK) {
			NMNAVICORE nvi;
			memset(&nvi, 0, sizeof(nvi));
			nvi.hdr.hwndFrom = m_hWnd;
			nvi.hdr.idFrom = GetDlgCtrlID();
			nvi.hdr.code = NVCN_GETDISPINFO;
			nvi.item.mask = NVCIF_IMAGE;
			nvi.item.iItem = index;
			nvi.item.iImage = -1;
			nvi.item.lParam = m_list.GetParam(index);
			GetParent().SendMessage(WM_NOTIFY, (WPARAM)nvi.hdr.idFrom, (LPARAM)&nvi);
			iImage = nvi.item.iImage;
			if (nvi.item.mask & NVCIF_DI_SETITEM)
				 m_list.SetImageIndex(index, iImage);
		}
		return iImage;
	}

	void OnMeasureItem(int idCtrl, LPMEASUREITEMSTRUCT pmi)
	{
		if (pmi->CtlType != ODT_MENU) return;

		CRect rc;
		GetClientRect(&rc);
		pmi->itemWidth = rc.Width() - GetSystemMetrics(SM_CXMENUCHECK) - GetSystemMetrics(SM_CXEDGE) * 2 - GetSystemMetrics(SM_CXBORDER) * 2;
		pmi->itemHeight = 1;
	}

	void OnDrawItem(int idCtrl, LPDRAWITEMSTRUCT pdi)
	{
		if (pdi->CtlType != ODT_MENU) return;

		CDCHandle dc = pdi->hDC;
		dc.FillSolidRect(&pdi->rcItem, GetSysColor(COLOR_MENU));
	}

	void TrackListMenu()
	{
		int cnt = m_list.GetSize();
		if (cnt >= 0) {
			// ボタン押下
			m_bPressed = true;
			InvalidateRect(&m_rTDrop, FALSE);

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
			HIMAGELIST hims, himl;
			Shell_GetImageLists(&himl, &hims);
			CSimpleArray<HBITMAP> arrbm;
			pop.AppendMenu(MF_OWNERDRAW, 1, (LPCTSTR)1);		// 幅を合わせるためのダミー項目
			for (int i = 0; i < cnt; i++) {
				pop.AppendMenu(0, IDM_NAVIHISTORY_FIRST + i, m_list[i]);
				int iImage = GetItemImageIndex(i);
				if (iImage >= 0) {
					HBITMAP hbm = CreateBitmapFromImageList(hims, iImage);
					if (hbm) {
						SetBitmapToMenuItem(pop, IDM_NAVIHISTORY_FIRST + i, hbm);
						arrbm.Add(hbm);
					}
				}
			}

			CRect rc;
			GetWindowRect(&rc);
			int cmd = pop.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, rc.left, rc.bottom, m_hWnd);

			while (arrbm.GetSize() > 0) {
				::DeleteObject(arrbm[0]);
				arrbm.RemoveAt(0);
			}

			// 選択文字列を編集ウィンドウへセット
			bool hereWeGo = false;
			int index = cmd - IDM_NAVIHISTORY_FIRST;
			if (index >= 0 && index < m_list.GetSize()) {
				m_indexSel = index;
				SetEditTextFromList(m_indexSel);
				hereWeGo = true;
			}

			// キャレット表示
			if ((info.flags & GUI_CARETBLINKING) && (info.hwndCaret == m_edit))
				::ShowCaret(info.hwndCaret);

			// ボタン戻し、アイコン再描画
			m_bPressed = false;
			Invalidate();

			if (hereWeGo)
				OnCommandGoTo();
		}
	}

	void OnCommandGoTo()
	{
		GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED), (LPARAM)m_hWnd);
	}

	void OnToolTipText(int id, NMHDR* pNmhdr, LRESULT* pResult)
	{
		*pResult = 0;
		if (GetCapture()) return;

		TOOLTIPTEXTW* pttt = (TOOLTIPTEXT*)pNmhdr;
		CString str;
		TCHAR text[MAX_PATH * 2];
		text[0] = 0;
		if (id == IDM_DIR_GOTO) {
			int cchMax = ARRAYSIZE(pttt->szText) - 6;
			int len = GetWindowText(str);
			if (len > 0) {
				if (len > cchMax)
					PathCompactPathEx(text, str, cchMax, 0);
				else
					lstrcpy(text, str);
				PathQuoteSpaces(text);
				lstrcat(text, _T(" へ移動"));
			}
		}

		// bring the tooltip window above other popup windows
		if (text[0]) {
			lstrcpyn(pttt->szText, text, ARRAYSIZE(pttt->szText));
			CToolTipCtrl tip = pNmhdr->hwndFrom;
			tip.SetMaxTipWidth(MY_INFOTIP_LEN);
		}
	}

	void OnNotifySetFocus(int id, NMHDR* pNmhdr, LRESULT* pResult)
	{
		*pResult = 0;
		if (id == IDC_EDIT_INNER) {
			DrawClientEdge();
		}
	}
	void OnNotifyKillFocus(int id, NMHDR* pNmhdr, LRESULT* pResult)
	{
		*pResult = 0;
		if (id == IDC_EDIT_INNER) {
			DrawClientEdge();
		}
	}
	BOOL OnEditCommandHandler(UINT code, UINT id, HWND hwndCtrl, BOOL& bHandled)
	{
//@		NMHDR hdr;
		switch (code) {
//		case EN_SETFOCUS:
//		case EN_KILLFOCUS: break;
//		case EN_ERRSPACE: break;
//		case EN_MAXTEXT: break;
//		case EN_HSCROLL: break;
//		case EN_VSCROLL: break;
		case SBN_ALTDOWN: 		OnListDropDown(); break;
		case SBN_DOWN:			OnListDown(); break;
		case SBN_UP:			OnListUp(); break;
/*
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
*/
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
			m_indexSel = ListFindString(-1, str, FALSE, FALSE);
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
				selNew = ListFindString(m_indexSel + 1, str, FALSE, FALSE);
				if (selNew < 0)
					selNew = m_indexSel + 1;
			}
		}

		if (m_indexSel != selNew) {
			m_indexSel = selNew;
			m_bEditChangeSkipOnce = true;
			m_edit.SetWindowText(m_list[m_indexSel]);
			m_edit.SetSel(0, -1);
			RedrawIconHeader();				// ヘッダアイコン再描画
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
				selNew = ListReverseFindString(m_indexSel - 1, str, FALSE, FALSE);
				if (selNew < 0)
					selNew = m_indexSel - 1;
			}
		}

		if (m_indexSel != selNew) {
			m_indexSel = selNew;
			m_bEditChangeSkipOnce = true;
			m_edit.SetWindowText(m_list[m_indexSel]);
			m_edit.SetSel(0, -1);
			RedrawIconHeader();				// ヘッダアイコン再描画
		}
	}

	void OnListDropDown()
	{
		if (m_bPressed)
			return;
		int numItems = m_list.GetSize();
		if (numItems < 1)
			return;

		if (m_list.GetSize() > 0) {
			TrackListMenu();
		}
	}

	int ListAddItem(LPCTSTR pszText, int iImage, LPARAM param)
	{
		m_list.Add(iImage, pszText, param);
		return m_list.GetSize();
	}

	BOOL ListDeleteItem(int index)
	{
		return m_list.Delete(index);
	}

	int ListResetContent()
	{
		m_list.DeleteAll();
		m_edit.SetWindowText(NULL);
		m_indexSel = -1;
		return 0;
	}

	int ListSetCurSel(int index)
	{
		if (index < 0 || index >= m_list.GetSize()) {
			m_edit.SetWindowText(NULL);
			return -1;
		}

		m_indexSel = index;
		m_bEditChangeSkipOnce = true;
		m_edit.SetWindowText(m_list[index]);
		m_edit.SetSel(0, -1);

		RedrawIconHeader();				// ヘッダアイコン再描画
		return 0;
	}

	int ListGetCurSel()
	{
		return m_indexSel;
	}

	int ListGetCount()
	{
		return m_list.GetSize();
	}

	int ListFindString(int indexStart, LPCTSTR pszFind, BOOL bExact, BOOL bWrapArround)
	{
		int numItems = m_list.GetSize();
		if (numItems < 1)
			return -1;

		if (indexStart < 0)
			indexStart = 0;
		if (indexStart >= numItems)
			return -1;

		if (!pszFind || *pszFind == 0)
			return -1;

		int len = lstrlen(pszFind);
		int iFound = -1;
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

	int ListReverseFindString(int indexStart, LPCTSTR pszFind, BOOL bExact, BOOL bWrapArround)
	{
		int numItems = m_list.GetSize();
		if (numItems < 1)
			return -1;

		if (indexStart < 0)
			indexStart = numItems - 1;
		if (indexStart >= numItems)
			return -1;

		if (!pszFind || *pszFind == 0)
			return -1;

		int len = lstrlen(pszFind);
		int iFound = -1;
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

	int ListSelectStringCore(int indexStart, LPCTSTR pszFind, BOOL bExact, BOOL bWrapArround)
	{
		int iFound = ListFindString(indexStart, pszFind, bExact, bWrapArround);
		if (iFound >= 0) {
			m_indexSel = iFound;
			m_bEditChangeSkipOnce = true;
			m_edit.SetWindowText(m_list[iFound]);
			m_edit.SetSel(0, -1);
			return -1;
		}
		return 0;
	}

	int ListSelectString(int indexStart, LPCTSTR pszFind)
	{
		return ListSelectStringCore(indexStart, pszFind, FALSE, TRUE);
	}

	int ListSelectStringExact(int indexStart, LPCTSTR pszFind)
	{
		return ListSelectStringCore(indexStart, pszFind, TRUE, TRUE);
	}

	void ListShowDropDown()
	{
		OnListDropDown();
	}

	void EditSetLimitText(UINT nMax)
	{
		m_edit.SetLimitText(nMax);
	}

	UINT EditGetLimitText() const
	{
		return m_edit.GetLimitText();
	}

	DWORD EditGetSel() const
	{
		return m_edit.GetSel();
	}

	void EditGetSel(int& nStartChar, int& nEndChar) const
	{
		m_edit.GetSel(nStartChar, nEndChar);
	}

	void EditSetSel(DWORD dwSelection, BOOL bNoScroll = FALSE)
	{
		m_edit.SetSel(dwSelection, bNoScroll);
	}

	void EditSetSel(int nStartChar, int nEndChar, BOOL bNoScroll = FALSE)
	{
		m_edit.SetSel(nStartChar, nEndChar, bNoScroll);
	}

	void EditSetReadOnly(bool bReadOnly)
	{
		m_edit.SetReadOnly(bReadOnly);
	}

	BOOL EditSetCueBannerText(LPCWSTR lpcwText)
	{
		return m_edit.SetCueBannerText(lpcwText);
	}

	BOOL EditGetCueBannerText(LPWSTR lpwText, int cchText) const
	{
		return m_edit.GetCueBannerText(lpwText, cchText);
	}

	HIMAGELIST GetImageList()
	{
		return m_hImage;
	}

	HIMAGELIST SetImageList(HIMAGELIST him)
	{
		HIMAGELIST htmp = m_hImage;
		m_hImage = him;

		if (!m_hImage)
			m_sizeImage = CSize(0, 0);
		else {
			int cx, cy;
			ImageList_GetIconSize(m_hImage, &cx, &cy);
			m_sizeImage = CSize(cx, cy);
		}

		if (IsWindow()) {
			CRect rc;
			GetClientRect(&rc);
			OnSize(0, rc.Width(), rc.Height());
		}
		return htmp;
	}

	LPARAM GetItemData(int index)
	{
		return m_list.GetParam(index);
	}

	CEdit GetEditCtrl() const
	{
		return CEdit(m_edit.m_hWnd);
	}

};

#endif		// __NAVICORE_H__
