///////////////////////////////////////////////////////
// url.cpp URL control implementation

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"

////////////////////////////////////////////////////////////////////////
//

COLORREF __fastcall PixelAlpha(COLORREF srcPixel, int percent)
{
	// My formula for calculating the transpareny is as
	// follows (for each single color):
	//
	//							   percent
	// destPixel = sourcePixel * ( ------- )
	//                               100
	//
	// This is not real alpha blending, as it only modifies the brightness,
	// but not the color (a real alpha blending had to mix the source and
	// destination pixels, e.g. mixing green and red makes yellow).
	// For our nice "menu" shadows its good enough.
	int r = GetRValue(srcPixel);
	if (r == 0) r++;
	int g = GetGValue(srcPixel);
	if (g == 0) g++;
	int b = GetBValue(srcPixel);
	if (b == 0) b++;

	return RGB ( min (255, (r * percent) / 100), 
				 min (255, (g * percent) / 100), 
				 min (255, (b * percent) / 100));
}

////////////////////////////////////////////////////////////////////////
//

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CLinkCtrlImpl : public CWindowImpl< T, TBase, TWinTraits>
{
protected:
	HFONT m_hFont;
	HIMAGELIST m_him;
	HICON m_hic;
	int m_gap;
	CSize m_sizeImage;
	COLORREF m_clrText;
	COLORREF m_clrHot;
	COLORREF m_clrClicked;
	COLORREF m_clrBack;
	bool m_bClicked:1;
	bool m_bHot:1;
	bool m_bHasFocus:1;
	bool m_bCustomColor:1;
	BOOL m_bTrackMouseEvent;

public:
	CLinkCtrlImpl()
	{
		m_hFont = GetStockFont(DEFAULT_GUI_FONT);
		m_him = NULL;
		m_hic = NULL;
		m_gap = 1;
		m_clrText = GetSysColor(COLOR_HIGHLIGHT);
		m_clrHot = PixelAlpha(m_clrText, 140);
		m_clrClicked = RGB(255, 0, 0);
		m_clrBack = COLOR_DEFAULT;
		m_bClicked = false;
		m_bHot = false;
		m_bHasFocus = false;
		m_bCustomColor = false;
		m_bTrackMouseEvent = FALSE;
	}

	~CLinkCtrlImpl()
	{
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

typedef CLinkCtrlImpl< T, TBase, TWinTraits >	thisClass;
BEGIN_MSG_MAP(this)
	MESSAGE_HANDLER(WM_GETDLGCODE, OnHandlerGetDlgCode)
	MESSAGE_HANDLER(WM_CREATE, OnHandlerCreate)
	MESSAGE_HANDLER(WM_SETFONT, OnHandlerSetFont)
	MESSAGE_HANDLER(WM_SETTEXT, OnHandlerSetText)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnHandlerEraseBkgnd)
	MESSAGE_HANDLER(WM_PAINT, OnHandlerPaint)
	MESSAGE_HANDLER(WM_PRINTCLIENT, OnHandlerPrintClient)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnHandlerLButtonDown)
	MESSAGE_HANDLER(WM_LBUTTONUP, OnHandlerLButtonUp)
	MESSAGE_HANDLER(WM_SETFOCUS, OnHandlerSetFocus)
	MESSAGE_HANDLER(WM_KILLFOCUS, OnHandlerKillFocus)
	MESSAGE_HANDLER(WM_SYSCOLORCHANGE, OnHandlerSysColorChange)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_MESSAGE(TLM_SETCOLORS, OnSetColors)
	ON_MESSAGE(TLM_SETBKCOLOR, OnSetBkColor)
	ON_MESSAGE(TLM_SETIMAGE, OnSetImage)
	ON_MESSAGE(TLM_GETIMAGE, OnGetImage)
	ON_MESSAGE(TLM_SETICON, OnSetIcon)
	ON_MESSAGE(TLM_GETICON, OnGetIcon)
END_MSG_MAP()

	LRESULT OnHandlerCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		return (LRESULT)pT->OnCreateClient((LPCREATESTRUCT)lParam);
	}

	LRESULT OnHandlerGetDlgCode(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		return pT->OnGetDlgCode();
	}

	LRESULT OnHandlerSetFont(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lr = DefWindowProc();
		m_hFont = wParam ? (HFONT)wParam : GetStockFont(DEFAULT_GUI_FONT);
		if (lParam)
			Invalidate(TRUE);
		return lr;
	}

	LRESULT OnHandlerSetText(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lr = DefWindowProc();
		Invalidate(TRUE);
		return lr;
	}

	LRESULT OnHandlerEraseBkgnd(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		return (LRESULT)pT->OnEraseBkgnd((HDC)wParam);
	}

	LRESULT OnHandlerPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CPaintDC dc(m_hWnd);
		T* pT = static_cast<T*>(this);
		pT->OnDraw(dc);
		return 0;
	}

	LRESULT OnHandlerPrintClient(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		if (lParam & PRF_ERASEBKGND)
			pT->OnEraseBkgnd((HDC)wParam);
		if (lParam & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED))
			pT->OnDraw((HDC)wParam);
		return 0;
	}

	LRESULT OnHandlerLButtonDown(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (!m_bClicked) {
			if (!m_bHasFocus)
				SetFocus();
			SetCapture();
			m_bClicked = true;
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		return 0;
	}

	LRESULT OnHandlerLButtonUp(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_bClicked) {
			ReleaseCapture();
			m_bClicked = false;
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

			CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			CRect rc;
			GetClientRect(&rc);
			if (rc.PtInRect(pt)) {
				T* pT = static_cast<T*>(this);
				pT->OnClick();
			}
		}
		return 0;
	}

	LRESULT OnHandlerSetFocus(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_bHasFocus = true;
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		return 0;
	}

	LRESULT OnHandlerKillFocus(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_bHasFocus = false;
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		return 0;
	}

	LRESULT OnHandlerSysColorChange(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (!m_bCustomColor) {
			T* pT = static_cast<T*>(this);
			pT->OnSysColorChange();
		}
		return 0;
	}

	void OnMouseMove(UINT flag, CPoint pt)
	{
		if (!m_bTrackMouseEvent) {
			TRACKMOUSEEVENT te;
			te.cbSize = sizeof(te);
			te.dwFlags = TME_LEAVE;
			te.hwndTrack = m_hWnd;
			te.dwHoverTime = HOVER_DEFAULT;
			m_bTrackMouseEvent = _TrackMouseEvent(&te);

			if (m_bTrackMouseEvent) {
				m_bHot = true;
				CClientDC dc(m_hWnd);
				T* pT = static_cast<T*>(this);
				pT->OnDraw(dc);
			}
		}
		DefWindowProc();
	}

	void OnMouseLeave()
	{
		if (m_bTrackMouseEvent) {
			m_bHot = false;
			CClientDC dc(m_hWnd);
			T* pT = static_cast<T*>(this);
			pT->OnDraw(dc);
		}
		m_bTrackMouseEvent = FALSE;
		DefWindowProc();
	}

	LRESULT OnSetColors(WPARAM wParam, LPARAM lParam)
	{
		TASKLINKCOLORS* puc = (TASKLINKCOLORS*)lParam;
		if (puc) {
			m_clrText = puc->clrText;
			m_clrHot = puc->clrFly;
			m_clrClicked = puc->clrClicked;
			m_bCustomColor = true;
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		else {
			T* pT = static_cast<T*>(this);
			pT->OnSysColorChange();
		}
		return 0;
	}

	virtual void OnSysColorChange()
	{
		m_clrText = GetSysColor(COLOR_HIGHLIGHT);
		m_clrHot = PixelAlpha(m_clrText, 140);
		m_clrClicked = RGB(255, 0, 0);
		m_bCustomColor = false;
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	LRESULT OnSetBkColor(WPARAM wParam, LPARAM lParam)
	{
		m_clrBack = (COLORREF)lParam;
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		return 0;
	}

	LRESULT OnSetImage(WPARAM wParam, LPARAM lParam)
	{
		m_hic = NULL;
		m_him = (HIMAGELIST)lParam;
		if (m_him) {
			int cx, cy;
			if (ImageList_GetIconSize(m_him, &cx, &cy))
				m_sizeImage = CSize(cx, cy);
			else
				m_him = NULL;
		}
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		return 0;
	}

	LRESULT OnGetImage(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)m_him;
	}

	LRESULT OnSetIcon(WPARAM wParam, LPARAM lParam)
	{
		m_him = NULL;
		m_hic = (HICON)lParam;
		if (m_hic) {
			ICONINFO ii;
			if (GetIconInfo(m_hic, &ii)) {
				BITMAP bm;
				GetObject(ii.hbmColor, sizeof(bm), &bm);
				m_sizeImage = CSize(bm.bmWidth, bm.bmHeight);
				if (ii.hbmColor) ::DeleteObject(ii.hbmColor);
				if (ii.hbmMask) ::DeleteObject(ii.hbmMask);
			}
			else
				m_hic = NULL;
		}
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		return 0;
	}

	LRESULT OnGetIcon(WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)m_hic;
	}

	virtual int OnCreateClient(LPCREATESTRUCT pcs)
	{
		return 0;
	}

	virtual LRESULT OnGetDlgCode()
	{
		return DLGC_BUTTON;
	}

	virtual BOOL OnEraseBkgnd(HDC hdc)
	{
		CRect rc;
		GetClientRect(&rc);
		HBRUSH hbr = CtlColor_Button(GetParent(), (WPARAM)hdc, m_hWnd);
		if (!hbr) hbr = GetSysColorBrush(COLOR_BTNFACE);
		FillRect(hdc, &rc, hbr);
		return TRUE;
	}

	virtual void OnDraw(HDC hdc)
	{
		CDCHandle dc = hdc;

		CRect rc;
		GetClientRect(&rc);
		CRect rcw(0, 0, rc.Width(), rc.Height());

		CDC dcw = CreateCompatibleDC(dc);
		HBITMAP hbm = CreateCompatibleBitmap(dc, rcw.Width(), rcw.Height());
		hbm = dcw.SelectBitmap(hbm);

		HBRUSH hbr;
		if (m_clrBack == COLOR_DEFAULT) {
			hbr = CtlColor_Button(GetParent(), dcw.m_hDC, m_hWnd);
			if (!hbr) hbr = GetSysColorBrush(COLOR_BTNFACE);
			dcw.FillRect(&rcw, hbr);
		}
		else
			dcw.FillSolidRect(&rcw, m_clrBack);

		CString str;
		if (GetWindowText(str) > 0) {
			DWORD style = GetStyle();

			LOGFONT lf;
			GetObject(m_hFont, sizeof(LOGFONT), &lf);
			if ((style & TLS_UNDERLINE) && m_bHot)
				lf.lfUnderline = TRUE;
			HFONT hfont = CreateFontIndirect(&lf);
			hfont = dcw.SelectFont(hfont);

			UINT flags = 0;
			if (style & TLS_CENTER)			flags |= DT_CENTER;
			if (style & TLS_RIGHT)			flags |= DT_RIGHT;
			if (style & TLS_VCENTER)		flags |= DT_VCENTER;
			if (style & TLS_BOTTOM)			flags |= DT_BOTTOM;
			if (style & TLS_SINGLELINE)		flags |= DT_SINGLELINE;
			if (style & TLS_PATH_ELLIPSIS)	flags |= DT_PATH_ELLIPSIS;
			if (style & TLS_END_ELLIPSIS)	flags |= DT_END_ELLIPSIS;

			int iImage = 0;
			COLORREF clr;
			if (!IsWindowEnabled()) {
				clr = GetSysColor(COLOR_GRAYTEXT);
				iImage = 2;
			}
			else {
				if (m_bClicked) {
					clr = m_clrClicked;
					iImage = 1;
				}
				else {
					if (m_bHot) {
						clr = m_clrHot;
						iImage = 1;
					}
					else {
						clr = m_clrText;
						iImage = 0;
					}
				}
			}

			// テキスト矩形を求める
			CRect rct(0, 0, 1, 1);
			dcw.DrawText(str, -1, &rct, flags | DT_CALCRECT);

			CRect rci;
			if (m_him || m_hic) {
				rci.left = 0;
				rci.right = rct.Width() + m_sizeImage.cx + 2 + m_gap * 2;
				rci.top = 0;
				rci.bottom = max(rct.Height(), m_sizeImage.cy) + m_gap * 2;

				if (style & TLS_CENTER)
					rci.OffsetRect((rcw.Width() - rci.Width()) / 2, 0);
				else if (style & TLS_RIGHT)
					rci.OffsetRect(rcw.Width() - rci.Width(), 0);

				if (style & TLS_VCENTER)
					rci.OffsetRect(0, (rcw.Height() - rci.Height()) / 2);
				else if (style & TLS_BOTTOM)
					rci.OffsetRect(0, rcw.Height() - rci.Height());

				if (m_him) {
					int cnt = ImageList_GetImageCount(m_him);
					iImage = min(iImage, cnt - 1);
					ImageList_Draw(m_him, iImage, dcw, m_gap + rci.left, rci.top + (rci.Height() - m_sizeImage.cy) / 2, ILD_TRANSPARENT);
				}
				else
					dcw.DrawIconEx(m_gap + rci.left, rci.top + (rci.Height() - m_sizeImage.cy) / 2, m_hic, m_sizeImage.cx, m_sizeImage.cy, 0, NULL, DI_NORMAL);

				rcw = rci;
				rcw.left += m_gap + m_sizeImage.cx + 2;
				rcw.right = rcw.left + rct.Width();
			}
			else {
				rci.left = 0;
				rci.right = rct.Width() + m_gap * 2;
				rci.top = 0;
				rci.bottom = rct.Height() + m_gap * 2;

				if (style & TLS_CENTER)
					rci.OffsetRect((rcw.Width() - rci.Width()) / 2, 0);
				else if (style & TLS_RIGHT)
					rci.OffsetRect(rcw.Width() - rci.Width(), 0);

				if (style & TLS_VCENTER)
					rci.OffsetRect(0, (rcw.Height() - rci.Height()) / 2);
				else if (style & TLS_BOTTOM)
					rci.OffsetRect(0, rcw.Height() - rci.Height());

				rcw = rci;
				rcw.DeflateRect(1, 1);
			}

			clr = dcw.SetTextColor(clr);
			dcw.SetBkMode(TRANSPARENT);
			dcw.DrawText(str, -1, &rcw, flags);
			dcw.SetTextColor(clr);

			if (m_bHasFocus)
				dcw.DrawFocusRect(&rci);

			DeleteObject(dcw.SelectFont(hfont));
		}

		// 実画面へ転送
		dc.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), dcw, 0, 0, SRCCOPY);
		DeleteObject(dcw.SelectBitmap(hbm));
	}

	virtual void OnClick()
	{
		GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), BN_CLICKED), (LPARAM)m_hWnd);
	}
};

////////////////////////////////////////////////////////////////////////
//

class CTaskLinkCtrl : public CLinkCtrlImpl<CTaskLinkCtrl>
{
public:
	CTaskLinkCtrl() { }
	~CTaskLinkCtrl() { }

BEGIN_MSG_MAP(CTaskLinkCtrl)
	CHAIN_MSG_MAP(CLinkCtrlImpl<CTaskLinkCtrl>)
END_MSG_MAP()

};

////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK TaskLinkWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CTaskLinkCtrl* pWnd = new CTaskLinkCtrl;
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}

////////////////////////////////////////////////////////////////////////
//

class CUrlCtrl : public CLinkCtrlImpl<CUrlCtrl>
{
public:
	CUrlCtrl() { }
	~CUrlCtrl() { }

BEGIN_MSG_MAP(CUrlCtrl)
	CHAIN_MSG_MAP(CLinkCtrlImpl<CUrlCtrl>)
END_MSG_MAP()

	void OnClick();
	void OnDraw(HDC hdc);
};

void CUrlCtrl::OnClick()
{
	CString str;
	if (GetWindowText(str) > 0) {
		ShellExecute(m_hWnd, _T("open"), str, NULL, _T(""), SW_NORMAL);
		GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), BN_CLICKED), (LPARAM)m_hWnd);
	}
}

void CUrlCtrl::OnDraw(HDC hdc)
{
	CDCHandle dc = hdc;

	CRect rc;
	GetClientRect(&rc);
	CRect rcw(0, 0, rc.Width(), rc.Height());

	CDC dcw = CreateCompatibleDC(dc);
	HBITMAP hbm = CreateCompatibleBitmap(dc, rcw.Width(), rcw.Height());
	hbm = dcw.SelectBitmap(hbm);

	COLORREF cr;
	HBRUSH hbr;
	if (m_clrBack == COLOR_DEFAULT) {
		hbr = CtlColor_Button(GetParent(), dcw.m_hDC, m_hWnd);
		if (!hbr) hbr = GetSysColorBrush(COLOR_BTNFACE);
		dcw.FillRect(&rcw, hbr);
	}
	else
		dcw.FillSolidRect(&rcw, m_clrBack);

	CString str;
	if (GetWindowText(str) > 0) {
		LOGFONT lf;
		GetObject(m_hFont, sizeof(LOGFONT), &lf);
		lf.lfUnderline = TRUE;
		HFONT hfont = CreateFontIndirect(&lf);
		hfont = dcw.SelectFont(hfont);

		if (!IsWindowEnabled())
			cr = GetSysColor(COLOR_GRAYTEXT);
		else {
			if (m_bClicked)	cr = m_clrClicked;
			else			cr = m_bHot ? m_clrHot : m_clrText;
		}
		cr = dcw.SetTextColor(cr);
		dcw.SetBkMode(TRANSPARENT);
		dcw.DrawText(str, -1, &rcw, DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
		dcw.SetTextColor(cr);

		DeleteObject(dcw.SelectFont(hfont));
	}

	// 実画面へ転送
	dc.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), dcw, 0, 0, SRCCOPY);
	DeleteObject(dcw.SelectBitmap(hbm));
}

////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK UrlWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CUrlCtrl* pWnd = new CUrlCtrl;
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}
