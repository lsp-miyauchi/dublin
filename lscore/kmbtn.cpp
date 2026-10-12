///////////////////////////////////////////////////////
// kmbtn.cpp

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"

#include "staticctrl.h"


///////////////////////////////////////////////////////
// STATIC control implementation

class CStaticCtrl : public CDefStaticCtrlImpl<CStaticCtrl>
{
public:
	BEGIN_MSG_MAP(CStaticCtrl)
		CHAIN_MSG_MAP(CDefStaticCtrlImpl<CStaticCtrl>)
	END_MSG_MAP()

	void OnDraw(HDC hdc)
	{
		CDCHandle dc = hdc;

		CRect rc, rcw;
		GetClientRect(&rcw);
		rc.CopyRect(&rcw);

		dc.FillRect(&rc, GetSysColorBrush(COLOR_BTNFACE));

		CString str;
		if (GetWindowText(str) > 0) {
			UINT flags;
			DWORD style = GetStyle();
			if      (style & SS_CENTER)	flags = DT_CENTER | DT_SINGLELINE;
			else if (style & SS_RIGHT)	flags = DT_RIGHT | DT_SINGLELINE;
			else						flags = DT_LEFT;
			if (style & SS_NOPREFIX)	flags |= DT_NOPREFIX;

			HFONT hfont = dc.SelectFont(m_hFont);
			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor(GetSysColor(COLOR_3DHILIGHT));
			dc.DrawText(str, -1, &rc, flags);
			rc.OffsetRect(1, 1);
			dc.SetTextColor(GetSysColor(IsWindowEnabled() ? COLOR_BTNTEXT : COLOR_BTNSHADOW));
			dc.DrawText(str, -1, &rc, flags);

			dc.SelectFont(hfont);
		}
	}
};

///////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK StaticWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CStaticCtrl* pWnd = new CStaticCtrl();
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}

///////////////////////////////////////////////////////
// RECT control implementation

#define ICON_GAP				4

class CRectCtrl : public CDefStaticCtrlImpl<CRectCtrl>
{
private:
	COLORREF m_clrFore;
	COLORREF m_clrBack;
	COLORREF m_clrEdge;
	HICON m_hIcon;
	CSize m_sizeIcon;

public:
	CRectCtrl() : m_hIcon(NULL)
	{
		m_sizeIcon = CSize(0, 0);
		m_clrFore = COLOR_DEFAULT;
		m_clrBack = COLOR_DEFAULT;
		m_clrEdge = COLOR_DEFAULT;
	}
	~CRectCtrl()
	{
		if (m_hIcon) DestroyIcon(m_hIcon);
	}

BEGIN_MSG_MAP(CRectCtrl)
	CHAIN_MSG_MAP(CDefStaticCtrlImpl<CRectCtrl>)
	ON_MESSAGE(WM_NCHITTEST, OnNcHitTest)
	ON_MESSAGE(RM_GETTEXTCOLOR, OnGetTextColor)
	ON_MESSAGE(RM_GETBKCOLOR, OnGetBkColor)
	ON_MESSAGE(RM_GETEDGECOLOR, OnGetEdgeColor)
	ON_MESSAGE(RM_SETTEXTCOLOR, OnSetTextColor)
	ON_MESSAGE(RM_SETBKCOLOR, OnSetBkColor)
	ON_MESSAGE(RM_SETEDGECOLOR, OnSetEdgeColor)
	ON_MESSAGE(RM_SETICON, OnSetIcon)
END_MSG_MAP()

	__inline LRESULT OnNcHitTest(WPARAM wParam, LPARAM lParam) { return HTTRANSPARENT; }
	__inline LRESULT OnGetTextColor(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_clrFore; }
	__inline LRESULT OnGetBkColor(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_clrBack; }
	__inline LRESULT OnGetEdgeColor(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_clrEdge; }
	LRESULT OnSetTextColor(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetBkColor(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetIcon(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetEdgeColor(WPARAM wParam, LPARAM lParam);

	int OnCreateClient(LPCREATESTRUCT pcs);
	void OnDraw(HDC hdc);
};

int CRectCtrl::OnCreateClient(LPCREATESTRUCT pcs)
{
	CRect rc;
	GetWindowRect(&rc);

	DWORD style = GetStyle();
	if (style & RS_HORZBORDER)		SetWindowPos(NULL, 0, 0, rc.Width(), 2, SWP_NOZORDER | SWP_NOMOVE);
	else if (style & RS_VERTBORDER)	SetWindowPos(NULL, 0, 0, 2, rc.Height(), SWP_NOZORDER | SWP_NOMOVE);
	return 0;
}

LRESULT CRectCtrl::OnSetTextColor(WPARAM wParam, LPARAM lParam)
{
	COLORREF clr = m_clrFore;
	if (m_clrFore != (COLORREF)lParam) {
		m_clrFore = (COLORREF)lParam;
		Invalidate(FALSE);
		UpdateWindow();
	}
	return (LRESULT)clr;
}

LRESULT CRectCtrl::OnSetBkColor(WPARAM wParam, LPARAM lParam)
{
	COLORREF clr = m_clrBack;
	if (m_clrBack != (COLORREF)lParam) {
		m_clrBack = (COLORREF)lParam;
		Invalidate(FALSE);
		UpdateWindow();
	}
	return (LRESULT)clr;
}

LRESULT CRectCtrl::OnSetEdgeColor(WPARAM wParam, LPARAM lParam)
{
	COLORREF clr = m_clrEdge;
	if (m_clrEdge != (COLORREF)lParam) {
		m_clrEdge = (COLORREF)lParam;
		Invalidate(FALSE);
		UpdateWindow();
	}
	return (LRESULT)clr;
}

LRESULT CRectCtrl::OnSetIcon(WPARAM wParam, LPARAM lParam)
{
	if (m_hIcon) {
		DestroyIcon(m_hIcon);
		m_sizeIcon = CSize(0, 0);
	}

	m_hIcon = lParam ? CopyIcon((HICON)lParam) : NULL;
	if (m_hIcon) {
		// アイコンのサイズを取得して、ウィンドウ矩形を調整する
		ICONINFO ii;
		GetIconInfo(m_hIcon, &ii);
		BITMAP bm;
		GetObject(ii.hbmColor, sizeof(bm), &bm);
		if (ii.hbmColor) DeleteObject(ii.hbmColor);
		if (ii.hbmMask) DeleteObject(ii.hbmMask);

		m_sizeIcon = CSize(bm.bmWidth, bm.bmHeight);

		int delta = (GetStyle() & RS_NOEDGE) ? 0 : 2;
		CRect rc;
		GetWindowRect(&rc);
		bool bNeedResize = false;
		if (rc.Width() < m_sizeIcon.cx + delta) {
			rc.right = rc.left + m_sizeIcon.cx + delta;
			bNeedResize = true;
		}
		if (rc.Height() < m_sizeIcon.cy + delta) {
			rc.bottom = rc.top + m_sizeIcon.cy + delta;
			bNeedResize = true;
		}
		if (bNeedResize) {
			::MapWindowPoints(HWND_DESKTOP, GetParent(), (LPPOINT)&rc, 2);
			MoveWindow(&rc, FALSE);
		}
	}

	Invalidate(FALSE);
	UpdateWindow();
	return 0;
}

void CRectCtrl::OnDraw(HDC hdc)
{
	CRect rc;
	GetClientRect(&rc);
	DWORD style = GetStyle();

	if (style & RS_OWNERDRAW) {
		DRAWITEMSTRUCT di;
		di.rcItem = rc;
		di.hDC = hdc;
		di.CtlType = 0;					// = ODT_STATIC;
		di.CtlID = GetDlgCtrlID();
		di.itemAction = ODA_DRAWENTIRE;
		di.itemState = 0;				// = ODS_DEFAULT;
		di.hwndItem = m_hWnd;
		di.itemData = 0;
		GetParent().SendMessage(WM_DRAWITEM, (WPARAM)di.CtlID, (LPARAM)&di);
		return;
	}

	CDCHandle dc = hdc;
	COLORREF clrShadow = GetSysColor(COLOR_3DSHADOW);
	COLORREF clrHilight = GetSysColor(COLOR_3DHILIGHT);
	if (style & RS_HORZBORDER) {
		dc.FillSolidRect(rc.left, rc.top, rc.Width(), 1, clrShadow);
		dc.FillSolidRect(rc.left, rc.top+1, rc.Width(), 1, clrHilight);
		return;
	}

	if (style & RS_VERTBORDER) {
		dc.FillSolidRect(rc.left, rc.top, 1, rc.Height(), clrShadow);
		dc.FillSolidRect(rc.left+1, rc.top, 1, rc.Height(), clrHilight);
		return;
	}

	if (!(style & RS_NOEDGE))
		rc.InflateRect(-1, -1);

	COLORREF crText = (m_clrFore == COLOR_DEFAULT) ? GetSysColor(COLOR_WINDOWTEXT) : m_clrFore;
	crText = dc.SetTextColor(crText);

	HBRUSH hbr;
	if (m_clrBack == COLOR_DEFAULT) {
		hbr = CtlColor_Static(GetParent(), hdc, m_hWnd);
		if (!hbr) hbr = GetSysColorBrush(COLOR_BTNFACE);
		dc.FillRect(&rc, hbr);
	}
	else
		dc.FillSolidRect(&rc, m_clrBack);

	// アイコン描画領域確保
	bool bHaveIcon = ((style & RS_ICON) && (m_hIcon != NULL));
	CRect rci = rc;

	TCHAR buf[MAX_PATH];
	if (GetWindowText(buf, ARRAYSIZE(buf)) > 0) {
		HFONT hfont = dc.SelectFont(m_hFont);

		// テキスト長を計算
		CRect rct(0, 0, 0, 0);
		dc.DrawText(buf, -1, &rct, DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
		int maxwidth = rct.Width();

		// アイコン分を加算
		if (bHaveIcon)
			maxwidth += m_sizeIcon.cx + ICON_GAP;

		int nMode = dc.SetBkMode(TRANSPARENT);
		UINT nFormat = DT_VCENTER | DT_SINGLELINE;
		if (style & RS_LEFT) {
			// 左寄せ
			nFormat |= DT_LEFT;
			if (bHaveIcon) {
				rc.left += m_sizeIcon.cx + ICON_GAP;
				rci.right = rc.left;
			}
		}
		else if (style & RS_RIGHT) {
			// 右寄せ
			nFormat |= DT_RIGHT;
			if (bHaveIcon) {
				// アイコンを持っている場合は位置を計算して左寄せで描画
				nFormat &= ~DT_RIGHT;
				nFormat |= DT_LEFT;
				if (rc.Width() > maxwidth) {
					// 入りきらないので左寄せ描画
					rc.left += m_sizeIcon.cx + ICON_GAP;
					rci.right = rc.left;
				}
				else {
					rci.left = rc.right - maxwidth;
					rci.right = rci.left + m_sizeIcon.cx + ICON_GAP;
					rc.left = rci.right;
				}
			}
		}
		else {
			// センタリング
			nFormat |= DT_CENTER;
			if (bHaveIcon) {
				// アイコンを持っている場合は位置を計算して左寄せで描画
				nFormat &= ~DT_CENTER;
				nFormat |= DT_LEFT;
				if (rc.Width() > maxwidth) {
					// 入りきらないので左寄せ描画
					rc.left += m_sizeIcon.cx + ICON_GAP;
					rci.right = rc.left;
				}
				else {
					rci.left = rc.left + (rc.Width() - maxwidth) / 2;
					rci.right = rci.left + m_sizeIcon.cx + ICON_GAP;
					rc.left = rci.right;
				}
			}
		}

		if (style & RS_ENDELLIPSIS)
			nFormat |= DT_END_ELLIPSIS;

		dc.DrawText(buf, -1, &rc, nFormat);
		dc.SetBkMode(nMode);
		dc.SelectFont(hfont);
	}

	if (bHaveIcon) {
		dc.DrawIconEx(
			rci.left + (rci.Width() - m_sizeIcon.cx) / 2,
			rci.top + (rci.Height() - m_sizeIcon.cy) / 2,
			m_hIcon, m_sizeIcon.cx, m_sizeIcon.cy, 0, NULL, DI_NORMAL);
	}
	dc.SetTextColor(crText);

	if (!(style & RS_NOEDGE)) {
		rc.InflateRect(1, 1);
		if (m_clrEdge == COLOR_DEFAULT)
			dc.DrawEdge(&rc, (style & RS_POP) ? BDR_RAISEDINNER : BDR_SUNKENOUTER, BF_TOPLEFT | BF_BOTTOMRIGHT);
		else
			draw_rect(dc, &rc, m_clrEdge, INVALID_COLOR);
	}
}

///////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK RectWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CRectCtrl* pWnd = new CRectCtrl();
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}

///////////////////////////////////////////////////////
// Signal control implementation

class CSignalCtrl : public CDefStaticCtrlImpl<CSignalCtrl>
{
private:
	HIMAGELIST m_him;
	BOOL m_busy;
	CSize m_sizeImage;

public:
	CSignalCtrl();
	~CSignalCtrl();

BEGIN_MSG_MAP(CSignalCtrl)
	CHAIN_MSG_MAP(CDefStaticCtrlImpl<CSignalCtrl>)
	ON_MESSAGE(WM_NCHITTEST, OnNcHitTest)
	ON_MESSAGE(SGM_GETSIGNAL, OnGetSignal)
	ON_MESSAGE(SGM_SETSIGNAL, OnSetSignal)
	ON_MESSAGE(SGM_SETIMAGELIST, OnSetImageList)
END_MSG_MAP()

	__inline LRESULT OnNcHitTest(WPARAM wParam, LPARAM lParam) { return HTTRANSPARENT; }
	__inline LRESULT OnGetSignal(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_busy; }
	LRESULT OnSetSignal(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetImageList(WPARAM wParam, LPARAM lParam);

	void OnDraw(HDC hdc);
};

CSignalCtrl::CSignalCtrl()
{
	m_busy = FALSE;
	m_him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_SIGNAL), 12, RGB(255, 0, 255));
	int cx, cy;
	ImageList_GetIconSize(m_him, &cx, &cy);
	m_sizeImage = CSize(cx, cy);
}

CSignalCtrl::~CSignalCtrl()
{
	if (m_him) ImageList_Destroy(m_him);
}

LRESULT CSignalCtrl::OnSetSignal(WPARAM wParam, LPARAM lParam)
{
	BOOL busy = !wParam ? FALSE : TRUE;
	if (m_busy != busy) {
		m_busy = busy;
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
	}
	return 0;
}

LRESULT CSignalCtrl::OnSetImageList(WPARAM wParam, LPARAM lParam)
{
	int cx, cy;
	if (ImageList_GetIconSize((HIMAGELIST)lParam, &cx, &cy)) {
		if (m_him) ImageList_Destroy(m_him);
		m_him = (HIMAGELIST)lParam;
		m_sizeImage = CSize(cx, cy);
		Invalidate();
		return 1;
	}
	return 0;
}

void CSignalCtrl::OnDraw(HDC hdc)
{
	if (m_him) {
		CRect rc;
		GetClientRect(&rc);
		int index = !IsWindowEnabled() ? 2 : m_busy ? 1 : 0;
		ImageList_Draw(m_him, index, hdc,
					rc.left + (rc.Width() - m_sizeImage.cx) / 2,
					rc.top + (rc.Height() - m_sizeImage.cy) / 2,
					ILD_TRANSPARENT);
	}
}

///////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK SignalWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CSignalCtrl* pWnd = new CSignalCtrl();
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}

///////////////////////////////////////////////////////
// COLOR control implementation

class CColorCtrl : public CDefStaticCtrlImpl<CColorCtrl>
{
private:
	COLORREF m_clr;
	COLORREF m_clrNearest;

public:
	CColorCtrl();
	~CColorCtrl() { }

BEGIN_MSG_MAP(CColorCtrl)
	CHAIN_MSG_MAP(CDefStaticCtrlImpl<CColorCtrl>)
	ON_WM_LBUTTONDBLCLK()
	ON_MESSAGE(CM_SETCOLORREF, OnSetColorRef)
	ON_MESSAGE(CM_GETCOLORREF, OnGetColorRef)
	ON_MESSAGE(CM_GETNEARESTCOLORREF, OnGetNearestColorRef)
END_MSG_MAP()

	void OnLButtonDblClk(UINT flag, CPoint pt);
	LRESULT OnSetColorRef(WPARAM wParam, LPARAM lParam);
	LRESULT OnGetColorRef(WPARAM wParam, LPARAM lParam);
	LRESULT OnGetNearestColorRef(WPARAM wParam, LPARAM lParam);

	BOOL OnEraseBkgnd(HDC hdc);
};

CColorCtrl::CColorCtrl()
{
	m_clr = m_clrNearest = RGB(0, 0, 0);
}

BOOL CColorCtrl::OnEraseBkgnd(HDC hdc)
{
	CDCHandle dc = hdc;
	CRect rc;
	GetClientRect(&rc);
	int i = rc.right / 2;
	rc.right = i;
	dc.FillSolidRect(&rc, m_clr);
	rc.OffsetRect(i, 0);
	dc.FillSolidRect(&rc, m_clrNearest);
	return TRUE;
}

void CColorCtrl::OnLButtonDblClk(UINT flag, CPoint pt)
{
	CRect rc;
	GetClientRect(&rc);
	rc.right /= 2;
	int id = GetDlgCtrlID();
	if (rc.PtInRect(pt))
		GetParent().PostMessage(WM_COMMAND, MAKEWPARAM(id, CN_SELCOLORREF), (LPARAM)m_hWnd);
	else
		GetParent().PostMessage(WM_COMMAND, MAKEWPARAM(id, CN_SELNEARESTCOLORREF), (LPARAM)m_hWnd);
}

LRESULT CColorCtrl::OnSetColorRef(WPARAM wParam, LPARAM lParam)
{
	m_clr = (COLORREF)lParam;
	CClientDC dc(m_hWnd);
	m_clrNearest = dc.GetNearestColor(m_clr);
	Invalidate(TRUE);
	UpdateWindow();
	return 0;
}

_inline LRESULT CColorCtrl::OnGetColorRef(WPARAM wParam, LPARAM lParam)
	{ return (LRESULT)m_clr; }

_inline LRESULT CColorCtrl::OnGetNearestColorRef(WPARAM wParam, LPARAM lParam)
	{ return (LRESULT)m_clrNearest; }

///////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK ColorWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CColorCtrl* pWnd = new CColorCtrl();
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}

