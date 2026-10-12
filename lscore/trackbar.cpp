///////////////////////////////////////////////////////
// TRACKBAR control implementation

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "muldiv32.h"
#include "fnc.h"
#include "common.h"
#include "trackbar.h"

#define ABS(a)				((a)>=0?(a):-(a))
#define ECR_TOPLEFT			RGB(152, 158, 148)
#define ECR_BOTTOMRIGHT		RGB(255, 255, 255)

////////////////////////////////////////////////////////////////////////////
//

CTrackCtrl::CTrackCtrl()
{
	m_min = 0;
	m_max = 100;
	m_pos = 0;
	m_step = 1;
	m_page = 10;
	m_freq = 10;
	m_selmin = m_selmax = 0;
	m_bCaptured = FALSE;
	m_bKeyPressed = FALSE;
	m_buddyBase = 0;
	m_hfont = GetStockFont(DEFAULT_GUI_FONT);
	m_him = m_himUser = NULL;
	m_suffix = NULL;
}

CTrackCtrl::~CTrackCtrl()
{
	if (m_him) ImageList_Destroy(m_him);
	if (m_suffix) LocalFree(m_suffix);
}

int CTrackCtrl::OnCreate(LPCREATESTRUCT pcs)
{
	m_sThumb = CSize(15, 12);
	m_him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_TRACK), m_sThumb.cx, RGB(255, 0, 255));
	return 0;
}

void CTrackCtrl::OnSetText(LPCTSTR pszText, BOOL& bHandled)
{
	DefWindowProc();
	Invalidate();
}

void CTrackCtrl::OnPrintClient(HDC hdc, LPARAM flags)
{
	if (flags & PRF_ERASEBKGND)
		return;
	if (flags & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED))
		OnDraw(hdc);
}

void CTrackCtrl::OnPaint()
{
	CPaintDC dc(m_hWnd);
	OnDraw(dc);
}

void CTrackCtrl::OnDraw(HDC hdc)
{
	BOOL enable = IsWindowEnabled();
	DWORD style = GetStyle();

	CRect rc, rcw;
	GetClientRect(&rc);

	CDCHandle dc = hdc;
	HBITMAP hbm = CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
	CDC dcw;
	dcw.CreateCompatibleDC(dc);
	hbm = dcw.SelectBitmap(hbm);

	CRect rcp =rc;
	rcp.DeflateRect(1, 1);
	dcw.FillSolidRect(&rcp, RGB(242, 241, 233));

	int d;
	if (enable && GetFocus() == m_hWnd) {
		d = (rc.Height() + 2) / 3;
		CRect w = rcp;
		w.bottom = w.top + d;
		FillShadeRect(dcw, w, RGB(224, 224, 224), RGB(242, 241, 233), TRUE);
		w = rcp;
		w.top = w.bottom - d - 1;
		FillShadeRect(dcw, w, RGB(242, 241, 233), RGB(224, 224, 224), TRUE);
	}

	if (style & TBS_ENABLESELRANGE && m_selmin != m_selmax) {
		int s1 = ((m_selmin - m_min) * rcp.Width()) / (m_max - m_min);
		int s2 = ((m_selmax - m_min) * rcp.Width()) / (m_max - m_min);

		CRect w = rcp;
		w.left = min(s1, s2);
		w.right = max(s1, s2);
		w.DeflateRect(0, 3);
		dc.FillSolidRect(&w, RGB(200, 200, 255));
	}

	int pos;
	int range = m_max - m_min;
	if (!(style & TBS_NOTICKS)) {
		HPEN hpen = dcw.SelectPen(CreatePen(PS_SOLID, 0, RGB(128, 128, 128)));
		double delta = (double)(rcp.Width() * m_step) / (double)range;
		pos = 1;
		for (int i = m_step; i < range; i += m_step) {
			if ((pos % m_freq) == 0) {
				d = (int)(delta * (double)i);
				dcw.MoveTo(d, 0);
				dcw.LineTo(d, 3);
			}
			pos++;
		}
		DeleteObject(dcw.SelectPen(hpen));
	}

	dcw.SetBkMode(TRANSPARENT);
	HFONT hf = dcw.SelectFont(m_hfont);
	dcw.SetTextColor(GetSysColor(enable ? COLOR_WINDOWTEXT : COLOR_GRAYTEXT));

	rcw = rc;
	rcw.DeflateRect(3, 0);
	TCHAR tit[MAX_PATH];
	if (GetWindowText(tit, ARRAYSIZE(tit)) > 0)
		dcw.DrawText(tit, -1, &rcw, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	pos = ABS(m_pos);
	TCHAR buf[MAX_PATH];
	TCHAR* pb = buf;
	if (m_pos < 0)
		*pb++ = '-';

	switch (m_buddyBase) {
		case 1:  wsprintf(pb, _T("%d.%d"), pos / 10, pos % 10); break;
		case 2:  wsprintf(pb, _T("%d.%02d"), pos / 100, pos % 100); break;
		case -1:
		{
			TRACKBARVALUE tv;
			memset(&tv, 0, sizeof(tv));
			tv.hdr.hwndFrom = m_hWnd;
			tv.hdr.idFrom = GetDlgCtrlID();
			tv.hdr.code = TRBN_GETVALUE;
			tv.pos = m_pos;
			if (GetParent().SendMessage(WM_NOTIFY, (WPARAM)tv.hdr.idFrom, (LPARAM)&tv)) {
				lstrcpy(buf, tv.szValue);
				break;
			}
		}
		default: wsprintf(pb, _T("%d"), pos); break;
	}
	if (m_suffix) wsprintf(&buf[lstrlen(buf)], _T(" %s"), m_suffix);

	dcw.DrawText(buf, -1, &rcw, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	dcw.SelectFont(hf);

	d = ((m_pos - m_min) * rcp.Width()) / range;
	HIMAGELIST him = m_himUser ? m_himUser : m_him;

	int y = (rcp.Height() - m_sThumb.cy + 1) / 2;
	if (!(style & TBS_NOTICKS))
		y++;
	ImageList_Draw(him, enable ? ((m_bCaptured || m_bKeyPressed) ? 1 : 0) : 2, dcw,
				d - (m_sThumb.cx >> 1), y,
				ILD_TRANSPARENT);

	// エッジの描画
	dcw.Draw3dRect(rc, ECR_TOPLEFT, ECR_BOTTOMRIGHT);

	dc.BitBlt(0, 0, rc.Width(), rc.Height(), dcw, 0, 0, SRCCOPY);
	DeleteObject(dcw.SelectBitmap(hbm));
}

void CTrackCtrl::OnMouseMove(UINT nFlags, CPoint pt)
{
	if (m_bCaptured) {
		CRect rc;
		GetClientRect(&rc);
		rc.DeflateRect(1, 1);

		int x = max(rc.left, min(rc.right, pt.x));
		int range = m_max - m_min;
		int pos = m_min + MulDivRN(x - rc.left, range, rc.Width());
		if (m_pos != pos) {
			SetPos(pos, TRUE);
			GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(TB_THUMBTRACK, m_pos), (LPARAM)m_hWnd);
		}
	}
}

void CTrackCtrl::OnCancelMode()
{
	if (!m_bCaptured)
		return;

	ReleaseCapture();
	m_bCaptured = FALSE;
}

void CTrackCtrl::OnLButtonDown(UINT nFlags, CPoint pt)
{
	if (GetFocus() != m_hWnd)
		SetFocus();

	CRect rc;
	GetClientRect(&rc);
	rc.DeflateRect(1, 1);
	pt.x -= rc.left;

	int range = m_max - m_min;
	int x = ((m_pos - m_min) * rc.Width()) / range;
	if (x - 3 > pt.x || pt.x > x + 3) {
		int pos = m_min + MulDivRN(pt.x, range, rc.Width());
		if (pos != m_pos) {
			SetPos(pos, TRUE);
			GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(TB_THUMBTRACK, m_pos), (LPARAM)m_hWnd);
		}
	}

	SetCapture();
	m_bCaptured = TRUE;

	CClientDC dc(m_hWnd);
	OnDraw(dc);
}

void CTrackCtrl::OnLButtonUp(UINT nFlags, CPoint pt)
{
	if (!m_bCaptured)
		return;

	ReleaseCapture();
	m_bCaptured = FALSE;

	CClientDC dc(m_hWnd);
	OnDraw(dc);

	GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(TB_THUMBPOSITION, m_pos), (LPARAM)m_hWnd);
}

BOOL CTrackCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	int pos = m_pos;
	UINT code;
	switch (nChar) {
	case VK_LEFT:
	case VK_DOWN:
		pos = max(m_min, pos - m_step);
		code = TB_LINEDOWN;
		break;

	case VK_RIGHT:
	case VK_UP:
		pos = min(m_max, pos + m_step);
		code = TB_LINEUP;
		break;

	case VK_PRIOR:
		pos = max(m_min, pos - m_page);
		code = TB_PAGEUP;
		break;

	case VK_NEXT:
		pos = min(m_max, pos + m_page);
		code = TB_PAGEDOWN;
		break;

	default:
		return FALSE;
	}

	if (pos != m_pos) {
		m_bKeyPressed = TRUE;
		m_pos = pos;
		Invalidate();
		UpdateWindow();
		GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(code, pos), (LPARAM)m_hWnd);
	}
	return TRUE;
}

BOOL CTrackCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar) {
	case VK_LEFT:
	case VK_DOWN:
	case VK_RIGHT:
	case VK_UP:
	case VK_PRIOR:
	case VK_NEXT:
		m_bKeyPressed = FALSE;
		Invalidate();
		UpdateWindow();
		GetParent().SendMessage(WM_HSCROLL, MAKEWPARAM(TB_ENDTRACK, m_pos), (LPARAM)m_hWnd);
		return TRUE;

	default:
		break;
	}
	return FALSE;
}

void CTrackCtrl::OnSize(UINT flags, int cx, int cy)
{
}

void CTrackCtrl::OnSetFont(HFONT hFont, BOOL bRedraw)
{
	if (!hFont)	m_hfont = GetStockFont(DEFAULT_GUI_FONT);
	else		m_hfont = hFont;

	if (bRedraw)
		Invalidate();
}

void CTrackCtrl::ChangeFocus(BOOL bHaveFocus)
{
	RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);

	NMHDR hdr;
	hdr.hwndFrom = m_hWnd;
	hdr.idFrom = GetDlgCtrlID();
	hdr.code = bHaveFocus ? NM_SETFOCUS : NM_KILLFOCUS;
	GetParent().SendMessage(WM_NOTIFY, (WPARAM)hdr.idFrom, (LPARAM)&hdr);
}

void CTrackCtrl::SetRange(int nMin, int nMax, BOOL bRedraw)
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

	int pos = max(nMin, min(m_pos, nMax));
	SetPos(pos, bRedraw);
}

void CTrackCtrl::SetSelection(int nMin, int nMax, BOOL bRedraw)
{
	if (GetStyle() & TBS_ENABLESELRANGE) {
		if (nMin > nMax) {
			int n = nMin;
			nMin = nMax;
			nMax = n;
		}
		m_selmin = nMin;
		m_selmax = nMax;

		if (bRedraw)
			Invalidate();
	}
}

void CTrackCtrl::SetPos(int pos, BOOL bRedraw)
{
	pos = max(m_min, min(pos, m_max));
	if (pos != m_pos) {
		m_pos = pos;
		if (bRedraw) {
			Invalidate();
			UpdateWindow();
		}
	}
}

int CTrackCtrl::SetPageSize(int page)
{
	int pageo = m_page;
	if (0 < page && page < (m_max - m_min))
		m_page = page;
	return pageo;
}

void CTrackCtrl::SetBuddyBase(int buddyBase)
{
	m_buddyBase = buddyBase;
	Invalidate();
}

void CTrackCtrl::SetTicFreq(int nFreq)
{
	if (m_freq != nFreq && nFreq > 0) {
		m_freq = nFreq;
		if (!(GetStyle() & TBS_NOTICKS)) Invalidate(FALSE);
	}
}

HIMAGELIST CTrackCtrl::SetThumbImage(HIMAGELIST him)
{
	if (ImageList_GetImageCount(him) < 3) return NULL;

	HIMAGELIST himOld = m_himUser;
	m_himUser = him;

	int cx, cy;
	if (m_himUser)	ImageList_GetIconSize(m_himUser, &cx, &cy);
	else			ImageList_GetIconSize(m_him, &cx, &cy);

	m_sThumb = CSize(cx, cy);
	Invalidate();

	return himOld;
}

void CTrackCtrl::SetSuffix(LPCTSTR pszSuffix)
{
	if (m_suffix) {
		LocalFree(m_suffix);
		m_suffix = NULL;
	}

	if (pszSuffix) {
		m_suffix = StrDup(pszSuffix);
		Invalidate();
	}
}

////////////////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK TrackWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CTrackCtrl* pWnd = new CTrackCtrl;
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}

