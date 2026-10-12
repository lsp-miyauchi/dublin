///////////////////////////////////////////////////////
// DEGREE control implementation

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "muldiv32.h"
#include "fnc.h"
#include "common.h"
#include "degree.h"

////////////////////////////////////////////////////////////////////////////
//

CDegreeCtrl::CDegreeCtrl()
{
	m_min = 0;
	m_max = 100;
	m_pos = 0;
	m_xofs = 1;
	m_step = 1;
	m_page = 10;
	m_bCaptured = FALSE;
	m_area.SetRectEmpty();
	m_arrColor.Add(RGB(0, 0, 0));
	m_arrColor.Add(RGB(255, 255, 255));
	m_buddyBase = 0;

	// Theme
	SetThemeClassList(L"Edit");
	SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE, 0);
}

void CDegreeCtrl::OnPrintClient(HDC hdc, LPARAM flags)
{
	if (flags & PRF_ERASEBKGND)
		OnEraseBkgnd(hdc);
	if (flags & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED))
		OnDraw(hdc);
}

void CDegreeCtrl::OnPaint()
{
	CPaintDC dc(m_hWnd);
	OnDraw(dc);
}

void CDegreeCtrl::OnDraw(HDC hdc)
{
	CDCHandle dc = hdc;
	int range = m_max - m_min;
	int pos = m_pos - m_min;
	int x = m_area.left + ((pos * m_area.Width()) / range) - (THUMBWIDTH >> 1);

	CRect rc(x, 0, x + THUMBWIDTH, m_area.Height());

	COLORREF crFace = GetSysColor(COLOR_3DFACE);
	COLORREF crShadow = GetSysColor(COLOR_3DSHADOW);
	COLORREF crHilite = GetSysColor(COLOR_3DHILIGHT);
	COLORREF crLite = GetSysColor(COLOR_3DLIGHT);
	COLORREF crDkshadow = GetSysColor(COLOR_3DDKSHADOW);

	if (GetStyle() & 1) {
		if (IsWindowEnabled() && GetFocus() == m_hWnd) {
			crFace = RGB(128, 128, 192);
			crHilite = LightenColor(crFace, 51);
			crLite = crFace;
			crShadow = DarkenColor(crFace, 51);
			crDkshadow = DarkenColor(crShadow, 51);
		}
	}

	if (m_bCaptured)
		crFace = DarkenColor(crFace, 31);

	CRect rci = rc;
	dc.Draw3dRect(rci, crHilite, crDkshadow);
	rci.DeflateRect(1, 1);
	dc.Draw3dRect(rci, crLite, crShadow);
	rci.DeflateRect(1, 1);

	dc.FillSolidRect(&rci, crFace);

	if (IsWindowEnabled()) {
		dc.FillSolidRect(rc.left + (THUMBWIDTH >> 1) - 1, rc.top + 2, 1, rc.Height() - 5, crShadow);
		dc.FillSolidRect(rc.left + (THUMBWIDTH >> 1), rc.top + 2, 1, rc.Height() - 5, crHilite);
	}
}

void CDegreeCtrl::OnMouseMove(UINT nFlags, CPoint pt)
{
	if (m_bCaptured) {
		pt.x -= m_xofs;
		int x = max(m_area.left, min(m_area.right, pt.x)) - m_area.left;
		int range = m_max - m_min;
		int pos = m_min + MulDivRN(x, range, m_area.Width());
		if (m_pos != pos) {
			SetPos(pos, TRUE);
			GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(TB_THUMBTRACK, m_pos), (LPARAM)m_hWnd);
		}
	}
}

void CDegreeCtrl::OnCancelMode()
{
	if (!m_bCaptured)
		return;

	ReleaseCapture();
	m_bCaptured = FALSE;
}

void CDegreeCtrl::OnLButtonDown(UINT nFlags, CPoint pt)
{
	if (GetFocus() != m_hWnd)
		SetFocus();

	int range = m_max - m_min;
	int pos = m_pos - m_min;
	int x = m_area.left + ((pos * m_area.Width()) / range);
	if (x - (THUMBWIDTH >> 1) <= pt.x && pt.x <= x + (THUMBWIDTH >> 1)) {
		SetCapture();
		m_bCaptured = TRUE;
		m_xofs = pt.x - x;

		CClientDC dc(m_hWnd);
		OnDraw(dc);
	}
	else {
		if (pt.x < x - (THUMBWIDTH >> 1))
			OnKeyDown(VK_PRIOR, 1, 0);
		else if (pt.x > x + (THUMBWIDTH >> 1))
			OnKeyDown(VK_NEXT, 1, 0);
	}
}

void CDegreeCtrl::OnLButtonUp(UINT nFlags, CPoint pt)
{
	if (!m_bCaptured)
		return;

	ReleaseCapture();
	m_bCaptured = FALSE;

	CClientDC dc(m_hWnd);
	OnDraw(dc);

	GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(TB_THUMBPOSITION, m_pos), (LPARAM)m_hWnd);
}

void CDegreeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	int pos = m_pos;
	UINT code;
	if (nChar == VK_LEFT || nChar == VK_DOWN) {
		pos = max(m_min, pos - m_step);
		code = TB_LINEDOWN;
	}
	else if (nChar == VK_RIGHT || nChar == VK_UP) {
		pos = min(m_max, pos + m_step);
		code = TB_LINEUP;
	}
	else if (nChar == VK_PRIOR) {
		pos = max(m_min, pos - m_page);
		code = TB_PAGEUP;
	}
	else if (nChar == VK_NEXT) {
		pos = min(m_max, pos + m_page);
		code = TB_PAGEDOWN;
	}
	else {
		DefWindowProc();
		return;
	}

	if (pos != m_pos) {
		SetPos(pos, TRUE);
		GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(code, pos), (LPARAM)m_hWnd);
	}
}

void CDegreeCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar) {
	case VK_LEFT:
	case VK_DOWN:
	case VK_RIGHT:
	case VK_UP:
	case VK_PRIOR:
	case VK_NEXT:
		GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(TB_ENDTRACK, m_pos), (LPARAM)m_hWnd);
		return;

	default:
		break;
	}
	DefWindowProc();
}

BOOL CDegreeCtrl::OnEraseBkgnd(HDC hdc)
{
	CDCHandle dc = hdc;
	CRect rc;
	GetClientRect(&rc);
	if (GetStyle() & 1) {
		dc.FillSolidRect(&rc, GetSysColor(COLOR_3DFACE));

		CRect rcw = rc;
		rcw.top = rc.top + (rc.Height() - 4) / 2;
		rcw.bottom = rcw.top + 4;

		dc.Draw3dRect(rcw, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT));

		if (IsWindowEnabled()) {
			rcw.DeflateRect(1, 1);
			dc.Draw3dRect(rcw, GetSysColor(COLOR_3DDKSHADOW), INVALID_COLOR);
		}
	}
	else if (GetStyle() & 2) {
		if (IsWindowEnabled()) {
			FillShadeRectEx(dc, rc, m_arrColor.GetData(), (int)m_arrColor.GetSize(), FALSE);
		}
		else {
			COLORREF rgb = dc.SetBkColor(GetSysColor(COLOR_3DFACE));
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
			dc.SetBkColor(rgb);
		}
	}
	else {
		if (IsWindowEnabled())
			dc.FillSolidRect(&rc,
				GetFocus() == m_hWnd ? RGB(184, 184, 220) : RGB(224, 224, 224));
		else {
			COLORREF rgb = dc.SetBkColor(GetSysColor(COLOR_3DFACE));
			dc.ExtTextOut(0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
			dc.SetBkColor(rgb);
		}
	}

	return TRUE;
}

void CDegreeCtrl::OnSize(UINT flags, int cx, int cy)
{
	m_area.SetRect(0, 0, cx, cy);
	m_area.DeflateRect(THUMBWIDTH >> 1, 0);
}

void CDegreeCtrl::ChangeFocus(BOOL bHaveFocus)
{
	RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);

	NMHDR hdr;
	hdr.hwndFrom = m_hWnd;
	hdr.idFrom = GetDlgCtrlID();
	hdr.code = bHaveFocus ? NM_SETFOCUS : NM_KILLFOCUS;
	GetParent().SendMessage(WM_NOTIFY, (WPARAM)hdr.idFrom, (LPARAM)&hdr);
}

void CDegreeCtrl::SetRange(int nMin, int nMax, BOOL bRedraw)
{
	if (nMin == nMax)
		return;

	if (nMin > nMax) {
		int n = nMin;
		nMin = nMax;
		nMax = n;
	}
	m_min = nMin;
	m_max = nMax;

	int pos = max(m_min, min(m_pos, m_max));
	SetPos(pos, bRedraw);
}

void CDegreeCtrl::SetPos(int pos, BOOL bRedraw)
{
	pos = max(m_min, min(pos, m_max));
	if (pos != m_pos) {
		m_pos = pos;
		if (bRedraw) {
			Invalidate();
			UpdateWindow();
		}
		SetValueToBuddy();
	}
}

int CDegreeCtrl::SetPageSize(int page)
{
	int pageo = m_page;
	if (0 < page && page < (m_max - m_min))
		m_page = page;
	return pageo;
}

void CDegreeCtrl::SetBkColors(COLORREF cr1, COLORREF cr2)
{
	m_arrColor.RemoveAll();
	m_arrColor.Add(cr1);
	m_arrColor.Add(cr2);
	if (GetStyle() & 2) {
		Invalidate();
		UpdateWindow();
	}
}

void CDegreeCtrl::SetBkColorsEx(COLORREF* crs, int numcolors)
{
	if (!crs || !numcolors)
		return;

	m_arrColor.RemoveAll();
	for (int i = 0; i < numcolors; i++)
		m_arrColor.Add(*crs++);
	if (GetStyle() & 2) {
		Invalidate();
		UpdateWindow();
	}
}

#define ABS(a)	((a)>=0?(a):-(a))

void CDegreeCtrl::SetValueToBuddy()
{
	if (m_buddy.IsWindow()) {
		int pos = ABS(m_pos);

		TCHAR buf[MAX_PATH];
		TCHAR* pb = buf;
		if (m_pos < 0)
			*pb++ = '-';

		if (m_buddyBase == 1)
			wsprintf(pb, _T("%d.%d"), pos / 10, pos % 10);
		else if (m_buddyBase == 2)
			wsprintf(pb, _T("%d.%02d"), pos / 100, pos % 100);
		else
			wsprintf(pb, _T("%d"), pos);
		m_buddy.SetWindowText(buf);
		m_buddy.UpdateWindow();
	}
}

HWND CDegreeCtrl::SetBuddy(HWND hBuddy)
{
	HWND hOldBuddy = m_buddy;
	if (::IsWindow(hBuddy)) {
		m_buddy = hBuddy;
		SetValueToBuddy();
	}
	return hOldBuddy;
}

void CDegreeCtrl::SetBuddyBase(int buddyBase)
{
	m_buddyBase = buddyBase;
	SetValueToBuddy();
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK DegreeWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CDegreeCtrl* pWnd = new CDegreeCtrl;
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}

