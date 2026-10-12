///////////////////////////////////////////////////////
// RANGEBAR control implementation

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "muldiv32.h"
#include "fnc.h"
#include "common.h"
#include "rangebar.h"

#define ABS(a)				((a)>=0?(a):-(a))
#define ECR_TOPLEFT			RGB(152, 158, 148)
#define ECR_BOTTOMRIGHT		RGB(255, 255, 255)

extern "C"
LRESULT CALLBACK RangeBarWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr;
	CRangeBar* pView;
	switch (msg) {
	case WM_NCCREATE:
		lr = DefWindowProc(hwnd, msg, wParam, lParam);
		pView = new CRangeBar();
		pView->Attach(hwnd);
		pView->m_parent = ((LPCREATESTRUCT)lParam)->hwndParent;
		SetWindowLongPtr(hwnd, 0, (LONG_PTR)pView);
		return lr;

	case WM_NCDESTROY:
		pView = (CRangeBar*)GetWindowLongPtr(hwnd, 0);
		SetWindowLongPtr(hwnd, 0, 0);
		delete pView;
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_GETDLGCODE:
		return DLGC_WANTARROWS;
	}

	pView = (CRangeBar*)GetWindowLongPtr(hwnd, 0);
	if (pView)
		return pView->WndProc(hwnd, msg, wParam, lParam);
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////
//

CRangeBar::CRangeBar()
{
	m_min = m_pos_min = 0;
	m_max = m_pos_max = 100;
	m_bCaptured = FALSE;
	m_him = NULL;
	m_bAlphaImage = FALSE;
}

CRangeBar::~CRangeBar()
{
	if (m_him)
		ImageList_Destroy(m_him);
}

LRESULT CRangeBar::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	m_msg.hwnd = hwnd;
	m_msg.message = msg;
	m_msg.wParam = wParam;
	m_msg.lParam = lParam;
	m_msg.time = GetMessageTime();
	m_msg.pt = CPoint(GetMessagePos());

	switch (msg) {
	case WM_CREATE:
		return OnCreate((LPCREATESTRUCT)lParam);

	case WM_SIZE:
		OnSize((UINT)wParam, (int)LOWORD(lParam), (int)HIWORD(lParam));
		return 0;

	case WM_SETFOCUS:
		OnChangeFocus(TRUE);
		return 0;

	case WM_KILLFOCUS:
		OnChangeFocus(FALSE);
		return 0;

	case WM_ENABLE:
		Invalidate();
 		return 0;

	case WM_ERASEBKGND:
		return TRUE;

	case WM_PRINTCLIENT:
	{
		CAttachDC dc((HDC)wParam);
		OnPrintClient(dc, lParam);
		return 0;
	}

	case WM_PAINT:
		OnPaint();
		return 0;

	case WM_LBUTTONDOWN:
		OnLButtonDown((UINT)wParam, CPoint(lParam));
		return 0;

	case WM_LBUTTONUP:
		OnLButtonUp((UINT)wParam, CPoint(lParam));
		return 0;

	case WM_MOUSEMOVE:
		OnMouseMove((UINT)wParam, CPoint(lParam));
		return 0;

	case WM_CANCELMODE:
		OnCancelMode();
		return 0;

	case TBM_SETRANGE:
		OnSetRange((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (BOOL)wParam);
		return 0;

	case TBM_SETPOS:
		OnSetPos((int)lParam, (BOOL)wParam);
		return 0;

	case TBM_GETPOS:
		return m_pos;

	default:
		break;
	}
	return Default();
}

int CRangeBar::OnCreate(LPCREATESTRUCT pcs)
{
	UINT id = IDB_TRACK;
	m_sThumb = CSize(15, 12);
	id = IsRGBA() ? IDB_TRACKBTN_XP : IDB_TRACKBTN;
	m_bAlphaImage = IsRGBA() ? lccIsAlphaBitmapResource(RESINST, MAKEINTRESOURCE(id)) : FALSE;
	m_him = ImageList_LoadImage(RESINST, MAKEINTRESOURCE(id), m_sThumb.cx, 0, RGB(255, 0, 255), IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_DEFAULTCOLOR);
	return Default();
}

void CRangeBar::OnSize(UINT flag, int cx, int cy)
{
	m_range = cy;
}

LRESULT CRangeBar::Default()
{
	return DefWindowProc(m_msg.hwnd, m_msg.message, m_msg.wParam, m_msg.lParam);
}

void CRangeBar::OnDraw(CDC& dc)
{
	DrawThumb(dc);
}

void CRangeBar::OnPrintClient(CDC& dc, LPARAM flags)
{
	if (flags & PRF_ERASEBKGND)
		return;

	if (flags & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED))
		OnDraw(dc);
}

void CRangeBar::OnPaint()
{
	CPaintDC dc(m_hWnd);
	OnDraw(dc);
}

void CRangeBar::DrawThumb(CDC& dc)
{
	CRect rc, rcw;
	GetClientRect(m_hWnd, &rc);

	if (!IsEnabled()) {
		dc.FillSolidRect(&rc, RGB(242, 241, 233));
		dc.Draw3dRect(rc, ECR_TOPLEFT, ECR_BOTTOMRIGHT);
		return;
	}

	HBITMAP hbm = CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
	CDC dcw;
	dcw.CreateCompatibleDC(&dc);
	hbm = (HBITMAP)dcw.SelectObject(hbm);

	rcw = rc;
	rcw.DeflateRect(1, 1);
	dcw.FillSolidRect(&rcw, RGB(242, 241, 233));

	int d;
	if (GetFocus() == m_hWnd) {
		d = (rc.Height() + 2) / 3;
		CRect w = rcw;
		w.bottom = w.top + d;
		FillShadeRect(dcw, w, RGB(224, 224, 224), RGB(242, 241, 233), TRUE);
		w = rcw;
		w.top = w.bottom - d - 1;
		FillShadeRect(dcw, w, RGB(242, 241, 233), RGB(224, 224, 224), TRUE);
	}

	ImageList_Draw(m_him, m_bCaptured ? 1 : 0, dcw,
				m_pos_min - (m_sThumb.cx >> 1),
				(rc.Height() - m_sThumb.cy + 1) / 2,
				ILD_TRANSPARENT);
	ImageList_Draw(m_him, m_bCaptured ? 1 : 0, dcw,
				m_pos_max - (m_sThumb.cx >> 1),
				(rc.Height() - m_sThumb.cy + 1) / 2,
				ILD_TRANSPARENT);

	// エッジの描画
	dcw.Draw3dRect(rc, ECR_TOPLEFT, ECR_BOTTOMRIGHT);

	dc.BitBlt(0, 0, rc.Width(), rc.Height(), &dcw, 0, 0, SRCCOPY);
	DeleteObject(dcw.SelectObject(hbm));
}

void CRangeBar::OnMouseMove(UINT nFlags, CPoint pt)
{
	if (m_bCaptured) {
		CRect rc;
		GetClientRect(m_hWnd, &rc);

		int x = max(rc.left, min(rc.right, pt.x));
		if (m_pDragPos != x) {
			if (m_pDragPos == &m_pos_min)
				x = min(m_pos_max - 1, x);
			else
				x = max(m_pos_min + 1, x);
			*m_pDragPos = x;
			CClientDC dc(m_hWnd);
			DrawThumb(dc);

			// 親へ通知
		}
	}
}

void CRangeBar::OnCancelMode()
{
	if (!m_bCaptured)
		return;

	ReleaseCapture();
	m_bCaptured = FALSE;
}

void CRangeBar::OnLButtonDown(UINT nFlags, CPoint pt)
{
	if (GetFocus() != m_hWnd)
		SetFocus(m_hWnd);

	if (pt.x <= m_pos_min)
		m_pDragPos = &m_pos_min;
	else if (pt.x >= m_pos_max)
		m_pDragPos = &m_pos_max;
	else
		m_pDragPos = ((m_pos_max + m_pos_min) / 2) > pt.x ? &m_pos_min : &m_pos_max;

	*m_pDragPos = pt.x;

	SetCapture(m_hWnd);
	m_bCaptured = TRUE;

	CClientDC dc(m_hWnd);
	DrawThumb(dc);

	// 親へ通知
}

void CRangeBar::OnLButtonUp(UINT nFlags, CPoint pt)
{
	if (!m_bCaptured)
		return;

	ReleaseCapture();
	m_bCaptured = FALSE;

	CClientDC dc(m_hWnd);
	DrawThumb(dc);

	// 親へ通知
}

void CRangeBar::OnChangeFocus(BOOL bHaveFocus)
{
	RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);

	NMHDR hdr;
	hdr.hwndFrom = m_hWnd;
	hdr.idFrom = GetDlgCtrlID(m_hWnd);
	hdr.code = bHaveFocus ? NM_SETFOCUS : NM_KILLFOCUS;
	m_parent.SendMessage(WM_NOTIFY, (WPARAM)hdr.idFrom, (LPARAM)&hdr);
}

void CRangeBar::OnSetRange(int nMin, int nMax, BOOL bRedraw)
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

	int pos = m_pos_min;
	if (pos < nMin)
		pos = nMin;
	else if (pos > nMax)
		pos = nMax;

	OnSetPos(pos, bRedraw);
}

void CRangeBar::OnSetLowPos(int pos, BOOL bRedraw)
{
	pos = max(m_min, min(pos, m_max));
	if (pos > m_pos_max)
		pos = m_pos_max;
	if (pos != m_pos_min) {
		m_pos_min = pos;
		if (bRedraw)
		{
			Invalidate();
			UpdateWindow(m_hWnd);
		}
	}
}

void CRangeBar::OnSetHighPos(int pos, BOOL bRedraw)
{
	pos = max(m_min, min(pos, m_max));
	if (pos < m_pos_min)
		pos = m_pos_min;
	if (pos != m_pos_max) {
		m_pos_max = pos;
		if (bRedraw)
		{
			Invalidate();
			UpdateWindow(m_hWnd);
		}
	}
}
