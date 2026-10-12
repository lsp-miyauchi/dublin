///////////////////////////////////////////////////////
// pophint.cpp

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"

extern "C" BOOL WINAPI lccAlphaBlend(HDC, int, int, int, int, HDC, int, int, DWORD);

///////////////////////////////////////////////////////
//


#define SYSCOLOR_BTNSHADOW		(GetSysColor(COLOR_BTNSHADOW))
#define SYSCOLOR_WINDOWFRAME	(GetSysColor(COLOR_WINDOWFRAME))
#define SYSCOLOR_INFOTEXT		(GetSysColor(COLOR_INFOTEXT))
#define SYSCOLOR_INFOBK			(GetSysColor(COLOR_INFOBK))

#define CXGAP					4
#define CYGAP					4
#define MAXTIPLINE				128


class CPopHintWindow :	public CWindowImpl<CPopHintWindow>,
						public CThemeImpl<CPopHintWindow>
{
private:
	int m_nLineHeight;
	int m_nCaptionHeight;
	CString m_strTipText;
	DWORD m_style;
	HICON m_hIcon;
	CSize m_sizeIcon;
	CSize m_sizeButton;
	UINT_PTR m_timerid;

public:
	DECLARE_WND_CLASS_EX(_T("PopHint"), CS_SAVEBITS, COLOR_WINDOW)

	CPopHintWindow(LPCTSTR pszText, HICON hIcon, DWORD style)
			: m_timerid(0)
			, m_nLineHeight(0)
			, m_nCaptionHeight(0)
			, m_strTipText(pszText)
			, m_style(style)
	{
		m_hIcon = (hIcon != NULL) ? CopyIcon(hIcon) : NULL;
		if (m_hIcon) {
			ICONINFO ii;
			GetIconInfo(m_hIcon, &ii);
			BITMAP bm;
			GetObject(ii.hbmColor, sizeof(bm), &bm);
			m_sizeIcon = CSize(bm.bmWidth, bm.bmHeight);
			if (ii.hbmColor) DeleteObject(ii.hbmColor);
			if (ii.hbmMask) DeleteObject(ii.hbmMask);
		}
		else {
			m_sizeIcon = CSize(0, 0);
		}

		m_sizeButton.cx = GetSystemMetrics(SM_CXSMSIZE) - 2;
		m_sizeButton.cy = GetSystemMetrics(SM_CYSMSIZE) - 2;

		SetThemeClassList(L"TOOLTIP");
	}

	~CPopHintWindow()
	{
		if (m_hIcon)
			DestroyIcon(m_hIcon);
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

	HWND Create(LPCTSTR pszCaption)
	{
		HWND hwnd = CWindowImpl::Create(NULL, CWindow::rcDefault, pszCaption,
									WS_POPUP | WS_VISIBLE,
									WS_EX_TOOLWINDOW | WS_EX_TOPMOST);
		ATLASSERT(hwnd != NULL);
		return hwnd;
	}

BEGIN_MSG_MAP(CPopHintWindow)
	CHAIN_MSG_MAP(CThemeImpl<CPopHintWindow>)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_PRINTCLIENT()
	MESSAGE_HANDLER(WM_TIMER, OnRelease)
	MESSAGE_HANDLER(WM_KEYDOWN, OnRelease)
	MESSAGE_HANDLER(WM_SYSKEYDOWN, OnRelease)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnRelease)
	MESSAGE_HANDLER(WM_MBUTTONDOWN, OnRelease)
	MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRelease)
END_MSG_MAP()

protected:
	int OnCreate(LPCREATESTRUCT pcs)
	{
		CClientDC dc(m_hWnd);
		HFONT hf = dc.SelectFont(GetStockFont(DEFAULT_GUI_FONT));
		TEXTMETRIC tm;
		GetTextMetrics(dc, &tm);
		m_nLineHeight = tm.tmHeight + tm.tmExternalLeading;

		LPCTSTR pszCaption = (pcs->lpszName && *pcs->lpszName) ? pcs->lpszName : NULL;
		m_nCaptionHeight = (pszCaption ? m_nLineHeight : 6) + CXGAP;

		CRect rc(0, 0, 640, 0);
		dc.DrawText(m_strTipText, -1, &rc, DT_NOPREFIX | DT_CALCRECT);
		dc.SelectFont(hf);

		// アイコン
		if (m_hIcon) {
			rc.right += m_sizeIcon.cx + CXGAP;
			if (rc.Height() < m_sizeIcon.cy)
				rc.bottom = rc.top + m_sizeIcon.cy;
		}

		// キャプションタイトル高さ
		if (pszCaption)
			rc.bottom += m_nCaptionHeight;
		// 「閉じる」ボタン高さ
		if (m_style & PVS_CLOSEBUTTON)
			rc.bottom += m_sizeButton.cy + CYGAP * 2;

		rc.right += CXGAP * 2;
		rc.bottom += CYGAP * 2;

		// ボーダーサイズの付加
		rc.right += 2;
		rc.bottom += 2;
		rc.right += CXGAP;
		rc.bottom += CXGAP;

		CPoint pt;
		GetCursorPos(&pt);
		pt.x = max(pt.x - (rc.right / 2), 0);
		pt.x = min(pt.x, GetSystemMetrics(SM_CXSCREEN) - rc.right);
		pt.y = max(pt.y - (rc.bottom / 2), 0);
		pt.y = min(pt.y, GetSystemMetrics(SM_CYSCREEN) - rc.bottom);

		MoveWindow(pt.x, pt.y, rc.Width(), rc.Height(), TRUE);

		SetCapture();
		if (!(m_style & PVS_CLOSEBUTTON))
			m_timerid = SetTimer(1, 10000, NULL);
		return 0;
	}

	BOOL OnEraseBkgnd(HDC hdc)
	{
		return TRUE;
	}

	void OnPaint()
	{
		CPaintDC dc(m_hWnd);
		OnDraw(dc);
	}

	void OnPrintClient(HDC hdc, DWORD flags)
	{
		if (flags & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED))
			OnDraw(hdc);
	}

	void ZapTimer(DWORD zapcount)
	{
		DWORD timeout = GetTickCount() + zapcount;
		MSG msg;
		while (timeout > GetTickCount()) {
			if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	LRESULT OnRelease(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_timerid) {
			KillTimer(m_timerid);
			m_timerid = 0;
		}

		ReleaseCapture();
		DestroyWindow();
		return 0;
	}

	void DrawCaption(HDC hdc, CRect& rc, LPCTSTR pszCaption)
	{
		CDCHandle dc = hdc;
		COLORREF clr1 = GetSysColor(COLOR_3DLIGHT);
		COLORREF clr2 = GetSysColor(COLOR_3DSHADOW);

		int y = rc.top + (rc.Height() - 6) / 2;
		if (y & 1)
			y--;
		dc.FillSolidRect(rc.left + 3, y++, rc.Width() - 6, 1, clr1);
		dc.FillSolidRect(rc.left + 3, y++, rc.Width() - 6, 1, clr2);
		dc.FillSolidRect(rc.left + 3, y++, rc.Width() - 6, 1, clr1);
		dc.FillSolidRect(rc.left + 3, y++, rc.Width() - 6, 1, clr2);
		dc.FillSolidRect(rc.left + 3, y++, rc.Width() - 6, 1, clr1);
		dc.FillSolidRect(rc.left + 3, y++, rc.Width() - 6, 1, clr2);

		if (pszCaption && *pszCaption) {
			TCHAR cap[MAX_PATH * 2];
			wsprintf(cap, _T("  %s  "), pszCaption);
			COLORREF clr = dc.SetBkColor(RGB(255, 255, 224));
			dc.DrawText(cap, -1, &rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
			dc.SetBkColor(clr);
		}
	}

	void DrawButton(HDC hdc, CRect& rc, int mode)
	{
		CDCHandle dc = hdc;
		CRect rcw = rc;
		rcw.DeflateRect(CXGAP, CYGAP);

		if (!IsThemeNull())
			DrawThemeBackground(dc, TTP_CLOSE, TTCS_NORMAL, &rcw, NULL);
		else
			dc.DrawFrameControl(&rcw, DFC_CAPTION, DFCS_CAPTIONCLOSE);
	}

	void DrawShadow(HDC hdc, CRect& rc)
	{
		CDCHandle dc = hdc;
		HBITMAP hbm = CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
		CDC dcw;
		dcw.CreateCompatibleDC(dc);
		hbm = dcw.SelectBitmap(hbm);

#ifdef USE_NO_GRADIENT_SHADOW
		dcw.PatBlt(0, 0, rc.Width(), rc.Height(), BLACKNESS);
#else
		CRect rcc(0, 0, rc.Width(), rc.Height());
		dcw.FillSolidRect(&rcc, RGB(192, 192, 192));
		rcc.DeflateRect(1, 1);
		dcw.FillSolidRect(&rcc, RGB(128, 128, 128));
		rcc.DeflateRect(1, 1);
		dcw.FillSolidRect(&rcc, RGB(64, 64, 64));
		rcc.DeflateRect(1, 1);
		dcw.FillSolidRect(&rcc, RGB(0, 0, 0));
#endif
		lccAlphaBlend(dc, rc.left, rc.top, rc.Width(), rc.Height(), dcw, 0, 0, 64);
		DeleteObject(dcw.SelectBitmap(hbm));
	}

	void OnDraw(HDC hdc)
	{
		CDCHandle dc = hdc;
		HFONT hfOld = dc.SelectFont(GetStockFont(DEFAULT_GUI_FONT));

		CRect rc;
		GetClientRect(&rc);
		rc.right -= CXGAP;
		rc.bottom -= CYGAP;

		// 影
		CRect rcw = rc;
		rcw.OffsetRect(CXGAP, CYGAP);
		DrawShadow(dc, rcw);

		// 本体
		if (!IsThemeNull())
			DrawThemeBackground(dc, TTP_STANDARD, TTSS_NORMAL, &rc, NULL);
		else {
			dc.FillSolidRect(rc.left, rc.top, rc.Width(), rc.Height(), SYSCOLOR_INFOBK);
			dc.FillSolidRect(rc.left, rc.top, rc.Width(), 1, SYSCOLOR_BTNSHADOW);
			dc.FillSolidRect(rc.left, rc.top, 1, rc.Height(), SYSCOLOR_BTNSHADOW);
			dc.FillSolidRect(rc.left, rc.bottom - 1, rc.Width(), 1, SYSCOLOR_WINDOWFRAME);
			dc.FillSolidRect(rc.right - 1, rc.top, 1, rc.Height(), SYSCOLOR_WINDOWFRAME);
		}
		rc.InflateRect(-1, -1);

		// キャプションタイトル
		dc.SetTextColor(SYSCOLOR_INFOTEXT);

		TCHAR buf[MAX_PATH * 2];
		if (GetWindowText(buf, ARRAYSIZE(buf)) > 0) {
			rcw = rc;
			rcw.bottom = rcw.top + m_nCaptionHeight;
			DrawCaption(dc, rcw, buf);
			rc.top += m_nCaptionHeight;
		}

		// アイコン
		if (m_hIcon) {
			dc.DrawIconEx(rc.left + CXGAP, rc.top + CYGAP, m_hIcon, 0, 0, 0, NULL, DI_NORMAL);
			rc.left += m_sizeIcon.cx + CXGAP;
		}

		// チップテキスト
		dc.SetBkMode(TRANSPARENT);
		rcw = rc;
		rcw.left += CXGAP;
		rcw.top += CYGAP;
		dc.DrawText(m_strTipText, -1, &rcw, DT_NOPREFIX);

		if (m_style & PVS_CLOSEBUTTON) {
			rcw = rc;
			rcw.left = rcw.right - (m_sizeButton.cx + CXGAP * 2);
			rcw.top = rcw.bottom - (m_sizeButton.cy + CYGAP * 2);
			DrawButton(dc, rcw, FALSE);
		}

		dc.SelectFont(hfOld);
	}
};


///////////////////////////////////////////////////////
//

extern "C"
void WINAPI lccPopViewEx(HWND hwnd, LPCTSTR pCaption, LPCTSTR pText, HICON hIcon, DWORD style)
{
	if (pText && *pText) {
		CPopHintWindow* hint = new CPopHintWindow(pText, hIcon, style);
		hint->Create(pCaption);
	}
}

extern "C"
void WINAPI lccPopView(HWND hwnd, LPCTSTR pCaption, LPCTSTR pText, DWORD style)
{
	lccPopViewEx(hwnd, pCaption, pText, NULL, style);
}

