///////////////////////////////////////////////////////
// CHECK control implementation

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"
#include "gdiplus_sup.h"

#define DF_BMP					0x0001
#define DF_TEXT					0x0002
#define IMAGEMASK(n)			(((n) >> 8) & 0xff)
#define STYLEMASK(n)			((n) & 0xff)


class CCheckButtonCtrl :	public CWindowImpl<CCheckButtonCtrl>,
							public CThemeImpl<CCheckButtonCtrl>
{
private:
	HIMAGELIST m_hImage;
	HFONT m_hFont;
	CSize m_sizeImage;
	BOOL m_bHot;
	BOOL m_bPressed;
	BOOL m_bChecked;
	BOOL m_bCaptured;
	BOOL m_bTrackMouseEvent;
	gdiplus_using m_gu;

public:
	CCheckButtonCtrl() :	m_hImage(NULL),
							m_bHot(FALSE),
							m_bPressed(FALSE),
							m_bChecked(FALSE),
							m_bCaptured(FALSE),
							m_bTrackMouseEvent(FALSE)
	{
		m_hFont = GetStockFont(DEFAULT_GUI_FONT);
		SetThemeClassList(L"Button");
	}

	~CCheckButtonCtrl()
	{
		if (m_hImage) ImageList_Destroy(m_hImage);
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

BEGIN_MSG_MAP(CCheckButtonCtrl)
	CHAIN_MSG_MAP(CThemeImpl<CCheckButtonCtrl>)
	ON_WM_CREATE()
	ON_MESSAGE(WM_GETDLGCODE, OnGetDlgCode)
	ON_MESSAGE(WM_SETTEXT, OnUpdateClient)
	ON_MESSAGE(WM_ENABLE, OnUpdateClient)
	ON_WM_SETFONT()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(BM_SETCHECK, OnSetCheck)
	ON_MESSAGE(BM_GETCHECK, OnGetCheck)
	ON_MESSAGE(BM_SETSTATE, OnUnsupportedCommand)
	ON_MESSAGE(BM_GETSTATE, OnUnsupportedCommand)
	ON_MESSAGE_VOID(BM_CLICK, OnButtonClick)
END_MSG_MAP()

	__inline LRESULT OnGetDlgCode(WPARAM wParam, LPARAM lParam) { return DLGC_BUTTON; }
	LRESULT OnUpdateClient(WPARAM wParam, LPARAM lParam);
	int OnCreate(LPCREATESTRUCT lpcs);
	void OnSetFont(HFONT hFont, BOOL bRedraw);
	void OnPaint();
	__inline void OnSetFocus(HWND hOldWnd) { ChangeFocus(); }
	__inline void OnKillFocus(HWND hNewWnd) { ChangeFocus(); }
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnLButtonDown(UINT nFlags, CPoint pt);
	void OnMouseMove(UINT nFlags, CPoint pt);
	void OnLButtonUp(UINT nFlags, CPoint pt);
	LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	BOOL OnEraseBkgnd(HDC hdc);
	LRESULT OnSetCheck(WPARAM wParam, LPARAM lParam);
	__inline LRESULT OnGetCheck(WPARAM wParam, LPARAM lParam) { return m_bChecked; }
	__inline LRESULT OnUnsupportedCommand(WPARAM wParam, LPARAM lParam) { return (LRESULT)-1; }
	void OnButtonClick(WPARAM wParam, LPARAM lParam);

	void DrawFace(HDC hdc, UINT nMode);
	void ChangeFocus();
};

int CCheckButtonCtrl::OnCreate(LPCREATESTRUCT lpcs)
{
	int id;
	switch (IMAGEMASK(GetStyle())) {
		case 1:
			id = IDB_CHECK2;
			break;
		case 2:
			id = IDB_CHECK3;
			break;
		default:
			id = IDB_CHECK;
			break;
	}

	// ビットマップサイズの取得
	m_sizeImage = CSize(12, 12);
	HBITMAP hbm = lccLoadImage(RESINST, RT_RCDATA, MAKEINTRESOURCE(id));
	if (hbm) {
		BITMAP bm;
		if (GetObject(hbm, sizeof(bm), &bm)) {
			m_sizeImage.cx = bm.bmWidth / 3;
			m_sizeImage.cy = bm.bmHeight;
		}
		::DeleteObject(hbm);
	}
	m_hImage = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(id), m_sizeImage.cx, RGB(255, 0, 255));
	return 0;
}

LRESULT CCheckButtonCtrl::OnUpdateClient(WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc();
	Invalidate(TRUE);
	return lr;
}

void CCheckButtonCtrl::OnSetFont(HFONT hFont, BOOL bRedraw)
{
	DefWindowProc();
	m_hFont = hFont;
	if (bRedraw)
		Invalidate(TRUE);
}

void CCheckButtonCtrl::OnPaint()
{
	CPaintDC dc(m_hWnd);
	DrawFace(dc, DF_BMP | DF_TEXT);
}

void CCheckButtonCtrl::OnButtonClick(WPARAM wParam, LPARAM lParam)
{
	if (STYLEMASK(GetStyle()) == BS_AUTOCHECKBOX) {
		m_bChecked ^= TRUE;
		CClientDC dc(m_hWnd);
		DrawFace(dc, DF_BMP);
		GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED), (LPARAM)m_hWnd);
	}
}

void CCheckButtonCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((nChar == VK_SPACE) & !m_bCaptured) {
		SetCapture();
		m_bPressed = TRUE;
		m_bCaptured = TRUE;

		CClientDC dc(m_hWnd);
		DrawFace(dc, DF_BMP);
	}
	else
		DefWindowProc();
}

void CCheckButtonCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_SPACE && m_bCaptured) {
		ReleaseCapture();
		m_bCaptured = FALSE;
		m_bPressed = FALSE;

		if (STYLEMASK(GetStyle()) == BS_AUTOCHECKBOX)
			m_bChecked ^= TRUE;

		CClientDC dc(m_hWnd);
		DrawFace(dc, DF_BMP);

		GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED), (LPARAM)m_hWnd);
	}
	else
		DefWindowProc();
}

void CCheckButtonCtrl::OnLButtonDown(UINT nFlags, CPoint pt)
{
	// フォーカスを持ってなければ取得する
	if (GetFocus() != m_hWnd) {
		SetFocus();
		ChangeFocus();
	}
	SetCapture();
	m_bPressed = TRUE;
	m_bCaptured = TRUE;

	CClientDC dc(m_hWnd);
	DrawFace(dc, DF_BMP);
}

void CCheckButtonCtrl::OnMouseMove(UINT nFlags, CPoint pt)
{
	if (m_bCaptured) {
		CRect rc;
		GetClientRect(&rc);

		// ウィンドウ矩形内にカーソルがあり、前回のカーソル位置が矩形内に無い ||
		// ウィンドウ矩形内にカーソルがなく、前回のカーソル位置が矩形内にある
		if ((rc.PtInRect(pt) && !m_bHot) || (!rc.PtInRect(pt) && m_bHot)) {
			m_bHot ^= TRUE;
			CClientDC dc(m_hWnd);
			DrawFace(dc, DF_BMP);
		}
	}
	else {
		if (!m_bTrackMouseEvent) {
			TRACKMOUSEEVENT te;
			te.cbSize = sizeof(te);
			te.dwFlags = TME_LEAVE;
			te.hwndTrack = m_hWnd;
			te.dwHoverTime = HOVER_DEFAULT;
			m_bTrackMouseEvent = _TrackMouseEvent(&te);
		}

		if (m_bTrackMouseEvent) {
			if (!m_bHot) {
				m_bHot = TRUE;
				CClientDC dc(m_hWnd);
				DrawFace(dc, DF_BMP);
			}
		}
	}
}

LRESULT CCheckButtonCtrl::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	m_bTrackMouseEvent = FALSE;
	m_bHot = FALSE;
	CClientDC dc(m_hWnd);
	DrawFace(dc, DF_BMP);
	return 0;
}

void CCheckButtonCtrl::OnLButtonUp(UINT nFlags, CPoint pt)
{
	if (m_bCaptured) {
		ReleaseCapture();
		m_bCaptured = FALSE;

		CRect rc;
		GetClientRect(&rc);
		if (rc.PtInRect(pt) && STYLEMASK(GetStyle()) == BS_AUTOCHECKBOX)
			m_bChecked ^= TRUE;

		m_bPressed = FALSE;
		CClientDC dc(m_hWnd);
		DrawFace(dc, DF_BMP);
		if (rc.PtInRect(pt))
			GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED), (LPARAM)m_hWnd);
	}
}

BOOL CCheckButtonCtrl::OnEraseBkgnd(HDC hdc)
{
	CRect rc;
	GetClientRect(&rc);
	HBRUSH hbr = CtlColor_Static(GetParent(), (WPARAM)hdc, m_hWnd);
	if (!hbr) hbr = GetSysColorBrush(COLOR_BTNFACE);
	FillRect(hdc, &rc, hbr);
	return TRUE;
}

LRESULT CCheckButtonCtrl::OnSetCheck(WPARAM wParam, LPARAM lParam)
{
	m_bChecked = wParam ? TRUE : FALSE;
	CClientDC dc(m_hWnd);
	DrawFace(dc, DF_BMP);
	return 0;
}

void CCheckButtonCtrl::DrawFace(HDC hdc, UINT nMode)
{
	CDCHandle dc = hdc;

	// ウィンドウがイネーブルかどうか取得
	BOOL bEnabled = IsWindowEnabled();
	HFONT hfont = dc.SelectFont(m_hFont);

	CRect rc;
	GetClientRect(&rc);

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);

	CSize sizeFrame;
	sizeFrame.cx = max(13, m_sizeImage.cx + 4);
	sizeFrame.cy = max(13, m_sizeImage.cy + 4);

	if (nMode & DF_BMP) {
		CRect rcb = rc;
		rcb.right = rc.left + sizeFrame.cx;
		rcb.top = (rc.top + rc.bottom - sizeFrame.cy) / 2;
		rcb.bottom = rcb.top + sizeFrame.cy;

		if (!IsThemeNull()) {
			// チェックマーク本体は自分で描くので、枠周りのみを処理する
			int iState;
			if (bEnabled) {
				if      (m_bPressed)				iState = CBS_UNCHECKEDPRESSED;
				else if (!m_bChecked && m_bHot)		iState = CBS_UNCHECKEDHOT;
				else								iState = CBS_UNCHECKEDNORMAL;
			}
			else
				iState = CBS_UNCHECKEDDISABLED;

			DrawThemeBackground(dc, BP_CHECKBOX, iState, &rcb, NULL);
			rcb.DeflateRect(2, 2);
		}
		else {
			dc.Draw3dRect(rcb, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT));
			rcb.DeflateRect(1, 1);
			dc.Draw3dRect(rcb, GetSysColor(COLOR_3DDKSHADOW), GetSysColor(COLOR_3DFACE));
			rcb.DeflateRect(1, 1);

			COLORREF cr;
			if (!bEnabled)
				cr = GetSysColor(COLOR_3DFACE);
			else {
				cr = GetSysColor(COLOR_WINDOW);
				if (m_bCaptured)
					cr = DarkenColor(cr, 31);
			}
			dc.FillSolidRect(&rcb, cr);
		}

		if (m_bChecked) {
			int index = bEnabled ? (m_bHot ? 1 : 0) : 2;
			ImageList_Draw(m_hImage, index, dc,
						rcb.left + (rcb.Width() - m_sizeImage.cx) / 2,
						rcb.top + (rcb.Height() - m_sizeImage.cy) / 2,
						ILD_TRANSPARENT);
		}
	}

	if (nMode & DF_TEXT) {
		rc.left += sizeFrame.cx + 3;

		CString str;
		int len = GetWindowTextLength();
		if (len > 0) {
			LPTSTR p = str.GetBuffer(len + 1);
			GetWindowText(p, len + 1);
			str.ReleaseBuffer();
		}
		else
			str = _T(" ");

		CRect rcw(0, 0, 0, 0);
		dc.DrawText(str, -1, &rcw, DT_SINGLELINE | DT_CALCRECT);
		rc.right  = min(rc.right, rc.left + rcw.right);
		rc.top    = (rc.top + rc.bottom - rcw.Height()) / 2;
		rc.bottom = rc.top + rcw.Height();

		dc.SetBkMode(TRANSPARENT);
		if (!bEnabled) {
			dc.SetTextColor(GetSysColor(COLOR_3DHILIGHT));
			rc.OffsetRect(1, 1);
			dc.DrawText(str, -1, &rc, DT_SINGLELINE);
			rc.OffsetRect(-1, -1);
			dc.SetTextColor(GetSysColor(COLOR_3DSHADOW));
		}
		dc.DrawText(str, -1, &rc, DT_SINGLELINE);

		rc.InflateRect(1, 1);
		COLORREF rgb = GetSysColor(COLOR_BTNFACE);
		dc.FillSolidRect(rc.left, rc.top, rc.Width(), 1, rgb);
		dc.FillSolidRect(rc.left, rc.bottom - 1, rc.Width(), 1, rgb);
		dc.FillSolidRect(rc.left, rc.top, 1, rc.Height(), rgb);
		dc.FillSolidRect(rc.right - 1, rc.top, 1, rc.Height(), rgb);

		if (GetFocus() == m_hWnd)
			dc.DrawFocusRect(&rc);
	}
	dc.SelectFont(hfont);
}

void CCheckButtonCtrl::ChangeFocus()
{
	CClientDC dc(m_hWnd);
	DrawFace(dc, DF_TEXT);
}

////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK CheckWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CCheckButtonCtrl* pWnd = new CCheckButtonCtrl;
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}
