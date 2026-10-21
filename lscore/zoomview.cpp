// zoomView.cpp : implementation of the CZoomView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <math.h>
#include "resource.h"
#include "lscore.h"
#include "zoomview.h"
#include "common.h"

static double zoomFactor[] = {
	0.1, 0.125, 0.1667, 0.2, 0.25, 0.3333, 0.4, 0.5, 0.6667, 0.8, 1.0,
	1.2, 1.3333, 1.5, 1.6667, 2.0, 2.3333, 2.6667, 3.0, 3.5, 4.0, 5.0, 6.0, 7.0, 8.0
};

const int numZoomFactor = sizeof(zoomFactor) / sizeof(double);

#ifdef MAX
#undef MAX
#endif
#ifdef MIN
#undef MIN
#endif

#ifndef _MKIR
#define _MKIR(id)				MAKEINTRESOURCE((id))
#endif

#define ZOOM_DEFAULT_INDEX		10
#define INNER_MARGIN			12
#define _PH						2
#define _PS						5
#define MAX(a,b)				((a) < (b) ? (b) : (a))
#define MIN(a,b)				((a) < (b) ? (a) : (b))
#define ROUND_INT(n)			((int)floor(0.5 + (n)))
#define FLOOR_INT(n)			((int)floor((n)))
#define CEIL_INT(n)				((int)ceil((n)))


CZoomView::CZoomView() : CScrollWindowImpl<CZoomView>()
	, m_bCentered(true)
	, m_bCaptured(false)
	, m_bAutoSizeClient(false)
	, m_nDrawingMode(ZVDM_BITMAP)
	, m_gdipBitmap(NULL)
	, m_hbm(NULL)
{
	m_crBkgnd = GetSysColor(COLOR_WINDOW);
	m_sizeImage = CSize(0, 0);
	m_grid = CSize(16, 16);
	m_gridType = PS_SOLID;
	m_crGrid = RGB(130, 130, 130);
	m_crGuid = RGB(74, 132, 255);

	m_zoomMode = ZoomGrab;
	m_zoomScale = zoomFactor[ZOOM_DEFAULT_INDEX];
	m_rcSel.SetRectEmpty();
	m_rcTrack.SetRectEmpty();

	// Load the zoom cursor
	HINSTANCE hInst = RESINST;
	m_hcZoomIn = ::LoadCursor(hInst, _MKIR(IC_ZOOMIN));
	m_hcZoomOut = ::LoadCursor(hInst, _MKIR(IC_ZOOMOUT));
	m_hcHandOpen = ::LoadCursor(hInst, _MKIR(IC_HANDOPEN));
	m_hcHandClose = ::LoadCursor(hInst, _MKIR(IC_HANDCLOSE));
	// system resource
	m_hcTLtoBR = ::LoadCursor(NULL, IDC_SIZENWSE);
	m_hcBLtoTR = ::LoadCursor(NULL, IDC_SIZENESW);
	m_hcVert = ::LoadCursor(NULL, IDC_SIZENS);
	m_hcHorz = ::LoadCursor(NULL, IDC_SIZEWE);
	m_hcMove = ::LoadCursor(NULL, IDC_SIZEALL);
	m_hcSel = ::LoadCursor(NULL, IDC_CROSS);
}

CZoomView::~CZoomView()
{
	if (m_hcZoomIn)		DestroyCursor(m_hcZoomIn);
	if (m_hcZoomOut)	DestroyCursor(m_hcZoomOut);
	if (m_hcHandOpen)	DestroyCursor(m_hcHandOpen);
	if (m_hcHandClose)	DestroyCursor(m_hcHandClose);
}

LRESULT CZoomView::OnHandlerCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetZoomMode((ZoomMode_)(GetStyle() & ZMVS_STYLE_MASK));
	return 0;
}

LRESULT CZoomView::OnHandlerSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	BOOL bTmp = TRUE;
	LRESULT lr = _baseClass::OnSize(uMsg, wParam, lParam, bTmp);

	if (wParam != SIZE_MINIMIZED) {
		if (m_bAutoSizeClient) {
			DoZoomFitClient();
		}
		else {
			NotifyVisibleChanged();
		}
	}

	return lr;
}

void CZoomView::ClearCachedImage()
{
	if (m_gdipBitmap) {
		Bitmap* bitmap = m_gdipBitmap;
		m_gdipBitmap = NULL;
		delete bitmap;
	}
	if (m_hbm) {
		HBITMAP hbm = m_hbm;
		m_hbm = NULL;
		DeleteObject(hbm);
	}
}

void CZoomView::OnDestroy()
{
	ClearCachedImage();				// HBITMAP, Bitmap* の削除とクリア

	HANDLE hdib = m_dib.Detach();
	if (hdib && !(GetStyle() & ZMVS_SHAREIMAGE))
		GlobalFree(hdib);
}

void CZoomView::ChangeFocus(BOOL bFocus)
{
	NMHDR hdr;
	hdr.hwndFrom = m_hWnd;
	hdr.idFrom = GetDlgCtrlID();
	hdr.code = bFocus ? NM_SETFOCUS : NM_KILLFOCUS;
	GetParent().SendMessage(WM_NOTIFY, (WPARAM)hdr.idFrom, (LPARAM)&hdr);
}

LRESULT CZoomView::OnHandlerGetImageSize(WPARAM wParam, LPARAM lParam)
{
	if (m_dib.isNull()) return (LRESULT)FALSE;

	SIZE* size = (SIZE*)lParam;
	if (size) {
		size->cx = m_sizeImage.cx;
		size->cy = m_sizeImage.cy;
	}
	return (LRESULT)TRUE;
}

HANDLE CZoomView::UpdateImage(HANDLE hdib, BOOL bRedraw)
{
	// 直前の情報
	CSize sizeDev;
	GetScrollSize(sizeDev);

	ClearCachedImage();				// HBITMAP, Bitmap* の削除とクリア

	HANDLE hdibOld = m_dib.Attach(hdib);

	m_sizeImage.cx = m_sizeImage.cy = 0;

	int width = 0;
	int height = 0;
	if (!m_dib.isNull()) {
		BITMAPINFOHEADER* pbh = (BITMAPINFOHEADER*)m_dib.BitmapInfo();
		m_sizeImage.cx = pbh->biWidth;
		m_sizeImage.cy = pbh->biHeight;;
		switch (m_nDrawingMode) {
			case ZVDM_GDIPLUS:
				m_gdipBitmap = new Bitmap(m_dib.BitmapInfo(), m_dib.FindBits());
				break;
			case ZVDM_BITMAP:
			{
				CClientDC dc(nullptr);
				m_hbm = m_dib.GetHBitmap(dc);
				break;
			}
			case ZVDM_DIB:
				break;
		}
		width = (int)(m_sizeImage.cx * m_zoomScale);
		height = (int)(m_sizeImage.cy * m_zoomScale);
	}

	if (sizeDev.cx != width || sizeDev.cy != height)
		SetScrollSize(width, height, TRUE);
	if (bRedraw)
		Invalidate();
	return hdibOld;
}

HANDLE CZoomView::SetImage(HANDLE hdib, BOOL bRedraw)
{
	// 直前の情報
	CSize sizeDev;
	GetScrollSize(sizeDev);

	ClearCachedImage();				// HBITMAP, Bitmap* の削除とクリア

	m_sizeImage.cx = m_sizeImage.cy = 0;
	SetScrollSize(1, 1, TRUE);

	HANDLE hdibOld = m_dib.Attach(hdib);

	int width = 0;
	int height = 0;
	if (!m_dib.isNull()) {
		BITMAPINFOHEADER* pbh = (BITMAPINFOHEADER*)m_dib.BitmapInfo();
		m_sizeImage.cx = pbh->biWidth;
		m_sizeImage.cy = pbh->biHeight;;
		switch (m_nDrawingMode) {
		case ZVDM_GDIPLUS:
			m_gdipBitmap = new Bitmap(m_dib.BitmapInfo(), m_dib.FindBits());
			break;
		case ZVDM_BITMAP:
		{
			CClientDC dc(nullptr);
			m_hbm = m_dib.GetHBitmap(dc);
			break;
		}
		case ZVDM_DIB:
			break;
		}

		width = ROUND_INT(m_sizeImage.cx * m_zoomScale);
		height = ROUND_INT(m_sizeImage.cy * m_zoomScale);
	}
	else {
//		m_zoomScale = 1.00;
		width = height = 1;
	}

	if (sizeDev.cx != width || sizeDev.cy != height) {
		SetScrollSize(width, height, TRUE);
		NotifyVisibleChanged();
	}
	if (bRedraw)
		Invalidate();
	return hdibOld;
}

COLORREF CZoomView::SetBkColor(BOOL bRedraw, COLORREF crNew)
{
	COLORREF cr = (COLORREF)(-1);
	if (m_crBkgnd != crNew) {
		cr = m_crBkgnd;
		m_crBkgnd = crNew;
		if (bRedraw)
			Invalidate(TRUE);
	}
	return cr;
}

void CZoomView::DoScroll(int nType, int nScrollCode, int& cxyOffset, int cxySizeAll, int cxySizePage, int cxySizeLine)
{
	CScrollWindowImpl::DoScroll(nType, nScrollCode, cxyOffset, cxySizeAll, cxySizePage, cxySizeLine);
	NotifyVisibleChanged();
}

void CZoomView::DoPaint(CDCHandle dc)
{
	if (m_dib.isNull()) return;

	CRect rccli;
	GetClientRect(&rccli);

	Rect rc(0, 0, m_sizeAll.cx, m_sizeAll.cy);
	if (m_bCentered) {
		if (rccli.Width() > m_sizeAll.cx)
			rc.X = (rccli.Width() - m_sizeAll.cx) / 2;
		if (rccli.Height() > m_sizeAll.cy)
			rc.Y = (rccli.Height() - m_sizeAll.cy) / 2;
	}

	switch (m_nDrawingMode) {
		case ZVDM_DIB:
			dc.SetStretchBltMode((GetZoomScale() < 1.0) ? HALFTONE : COLORONCOLOR);
			dc.StretchDIBits(rc.X, rc.Y, rc.Width, rc.Height, 0, 0, m_sizeImage.cx, m_sizeImage.cy,
							m_dib.FindBits(), m_dib.BitmapInfo(), DIB_RGB_COLORS, SRCCOPY);
			break;

		case ZVDM_BITMAP:
		{
			CDC dcw;
			dcw.CreateCompatibleDC(dc);
			HBITMAP ho = dcw.SelectBitmap(m_hbm);
			int bltmode = (GetZoomScale() < 1.0) ? HALFTONE : COLORONCOLOR;
			if (m_dib.NumBits() < 2) bltmode = BLACKONWHITE;

			dc.SetStretchBltMode((GetZoomScale() < 1.0) ? HALFTONE : COLORONCOLOR);
			dc.StretchBlt(rc.X, rc.Y, rc.Width, rc.Height, dcw, 0, 0, m_sizeImage.cx, m_sizeImage.cy, SRCCOPY);

			dcw.SelectBitmap(ho);
			break;
		}

		case ZVDM_GDIPLUS:
		default:
		{
			Graphics g((HDC)dc);
			InterpolationMode im = (m_zoomScale >= 1.0) ? InterpolationModeNearestNeighbor : InterpolationModeDefault;
			g.SetInterpolationMode(im);
			g.SetPixelOffsetMode(PixelOffsetModeHalf);
			g.DrawImage(m_gdipBitmap, rc);
			break;
		}
	}

	// グリッド
	if ((GetStyle() & ZMVS_GRID) && m_grid.cx > 0 && m_grid.cy > 0) {
		LOGBRUSH lb = { 0 };
		lb.lbStyle = BS_SOLID;
		lb.lbColor = m_crGrid;
		HPEN hpen = ExtCreatePen(PS_COSMETIC | PS_ALTERNATE, 1, &lb, 0, NULL);
		hpen = dc.SelectPen(hpen);

		const auto width = m_sizeImage.cx;
		const auto height = m_sizeImage.cy;
		CRect rcl;
		for (int x = 0; x < width; x += m_grid.cx) {
			const auto dx = ROUND_INT(x * m_zoomScale);
			dc.MoveTo(rc.X + dx, rc.Y);
			dc.LineTo(rc.X + dx, rc.Y + rc.Height);
		}

		for (int y = 0; y < height; y += m_grid.cy) {
			const auto dy = ROUND_INT(y * m_zoomScale);
			dc.MoveTo(rc.X, rc.Y + dy);
			dc.LineTo(rc.X + rc.Width, rc.Y + dy);
		}

		DeleteObject(dc.SelectPen(hpen));
	}

	// 選択枠
	if (m_rcSel.left || m_rcSel.right || m_rcSel.top || m_rcSel.bottom)
		DrawSelect(dc, m_rcSel);
}

void CZoomView::DrawSelect(HDC hdc, CRect& rc)
{
	CDCHandle dc = hdc;
	int rop = dc.SetROP2(R2_NOTXORPEN);
	HPEN hpen = CreatePen(PS_SOLID,  0, RGB(0, 0, 0)); // 0 width = 1 device unit
	hpen = dc.SelectPen(hpen);

	// save viewport
	CPoint ptViewportOrg;
	dc.GetViewportOrg(&ptViewportOrg);
	dc.SetViewportOrg(0, 0);

	// Draw the rect with lines (eliminate rect middle fill)
	CRect rcs = rc;
	rcs.NormalizeRect();
	DevLog(rcs);
	dc.Rectangle(&rcs);

	if (rcs.Width() > _PS && rcs.Height() > _PS) {
		dc.PatBlt(rcs.left  - _PH,     rcs.top    - _PH,     _PS, _PS, DSTINVERT);
		dc.PatBlt(rcs.right - _PH - 1, rcs.top    - _PH,     _PS, _PS, DSTINVERT);
		dc.PatBlt(rcs.left  - _PH,     rcs.bottom - _PH - 1, _PS, _PS, DSTINVERT);
		dc.PatBlt(rcs.right - _PH - 1, rcs.bottom - _PH - 1, _PS, _PS, DSTINVERT);

		int ct = rcs.left + rcs.Width() / 2;
		if (rcs.Width() > _PS * 2) {
			dc.PatBlt(ct - _PH, rcs.top    - _PH,     _PS, _PS, DSTINVERT);
			dc.PatBlt(ct - _PH, rcs.bottom - _PH - 1, _PS, _PS, DSTINVERT);
		}

		ct = rcs.top + rcs.Height() / 2;
		if (rcs.Height() > _PS * 2) {
			dc.PatBlt(rcs.left  - _PH,     ct - _PH, _PS, _PS, DSTINVERT);
			dc.PatBlt(rcs.right - _PH - 1, ct - _PH, _PS, _PS, DSTINVERT);
		}
	}

	// restore viewport
	dc.SetViewportOrg(ptViewportOrg);

	DeleteObject(dc.SelectPen(hpen));
	dc.SetROP2(rop);
}

void CZoomView::OnPaletteChanged(WPARAM wParam, LPARAM lParam)
{
}

LRESULT CZoomView::OnQueryNewPalette(WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

BOOL CZoomView::OnEraseBkgnd(HDC hdc)
{
	CRect rc;
	GetClientRect(&rc);

	CDCHandle dc = hdc;
	int save = dc.SaveDC();
	if (!m_dib.isNull()) {
		CRect rca(0, 0, m_sizeAll.cx, m_sizeAll.cy);

		CPoint pt;
		GetDeviceScrollOffset(pt);
		rca.OffsetRect(-pt.x, -pt.y);

		rca &= rc;
		dc.ExcludeClipRect(&rca);
	}
	dc.FillSolidRect(&rc, m_crBkgnd);
	dc.RestoreDC(save);
	return TRUE;
}

LRESULT CZoomView::OnKeyMessages(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UINT nChar = (UINT)wParam;
	UINT nFlags = (UINT)HIWORD(lParam);

	if (m_zoomMode == ZoomGrab) {
		if (nChar == VK_CONTROL) {
			if (nFlags & KF_UP)
				::SetCursor(m_hcHandOpen);
			else if (!(nFlags & KF_REPEAT))
				::SetCursor(m_hcZoomIn);
		}
	}
	else if (m_zoomMode == ZoomIn) {
		if (nChar == VK_SHIFT) {
			if (nFlags & KF_UP)
				::SetCursor(m_hcZoomIn);
			else if (!(nFlags & KF_REPEAT))
				::SetCursor(m_hcZoomOut);
		}
	}

	bHandled = FALSE;
	return 0;
}

void CZoomView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	NMKEY key;
	key.hdr.hwndFrom = m_hWnd;
	key.hdr.idFrom = GetDlgCtrlID();
	key.hdr.code = NM_KEYDOWN;
	key.nVKey = nChar;
	key.uFlags = nFlags;
	if (GetParent().SendMessage(WM_NOTIFY, (WPARAM)key.hdr.idFrom, (LPARAM)&key))
		return;

	if (m_dib.isNull()) return;


	// 選択矩形のキーボードによる移動
	bool bControl = (GetKeyState(VK_CONTROL) < 0);
	bool bShift = (GetKeyState(VK_SHIFT) < 0);
	if ((m_zoomMode == ZoomSel && !m_rcSel.IsRectEmpty()) && (bControl || bShift)) {
		int delta;
		if      (bControl && bShift)	delta = 16;
		else if (bShift)				delta = 8;
		else							delta = 1;

		CRect rc = m_rcSel;
		if (nChar == VK_UP) {
			if (rc.top > 0) {
				delta = min(delta, rc.top);
				rc.OffsetRect(0, -delta);
			}
		}
		else if (nChar == VK_DOWN) {
			if (rc.bottom < m_sizeImage.cy - 1) {
				delta = min(delta, m_sizeImage.cy - 1 - rc.bottom);
				rc.OffsetRect(0, delta);
			}
		}
		else if (nChar == VK_LEFT) {
			if (rc.left > 0) {
				delta = min(delta, rc.left);
				rc.OffsetRect(-delta, 0);
			}
		}
		else if (nChar == VK_RIGHT) {
			if (rc.right < m_sizeImage.cx - 1) {
				delta = min(delta, m_sizeImage.cx - 1 - rc.right);
				rc.OffsetRect(delta, 0);
			}
		}

		if (rc != m_rcSel) {
			CClientDC dc(m_hWnd);

			DrawSelect(dc, m_rcSel);
			m_rcSel = rc;
			DrawSelect(dc, m_rcSel);

			NotifySelectionChanged();
			return;
		}
	}

	switch (nChar) {
		case VK_HOME:	::SendMessage(m_hWnd, WM_VSCROLL, SB_TOP, 0);		break;
		case VK_END:	::SendMessage(m_hWnd, WM_VSCROLL, SB_BOTTOM, 0);	break;
		case VK_UP:		::SendMessage(m_hWnd, WM_VSCROLL, SB_LINEUP, 0);	break;
		case VK_DOWN:	::SendMessage(m_hWnd, WM_VSCROLL, SB_LINEDOWN, 0);	break;
		case VK_PRIOR:	::SendMessage(m_hWnd, WM_VSCROLL, SB_PAGEUP, 0);	break;
		case VK_NEXT:	::SendMessage(m_hWnd, WM_VSCROLL, SB_PAGEDOWN, 0);	break;
		case VK_LEFT:	::SendMessage(m_hWnd, WM_HSCROLL, SB_LINELEFT, 0);	break;
		case VK_RIGHT:	::SendMessage(m_hWnd, WM_HSCROLL, SB_LINERIGHT, 0);	break;
		default:		DefWindowProc(); break;
	}
}

void CZoomView::GetDeviceScrollOffset(CPoint& pt)
{
	GetScrollOffset(pt);

	CRect rc;
	GetClientRect(&rc);
	if (m_bCentered) {
		if (rc.Width() > m_sizeAll.cx)	pt.x = -(rc.Width() - m_sizeAll.cx) / 2;
		if (rc.Height() > m_sizeAll.cy)	pt.y = -(rc.Height() - m_sizeAll.cy) / 2;
	}
}

void CZoomView::LogDev(POINT* ppt, int cnt)
{
	if (cnt < 1) return;

	CPoint pt;
	GetDeviceScrollOffset(pt);
	for (int i = 0; i < cnt; i++) {
		ppt[i].x += pt.x;
		ppt[i].y += pt.y;
		if (m_zoomScale != 1.0) {
			ppt[i].x = ROUND_INT(ppt[i].x / m_zoomScale);
			ppt[i].y = ROUND_INT(ppt[i].y / m_zoomScale);
		}
	}
}

void CZoomView::PixelPointToScaledPoint(POINT* ppt, int cnt)
{
	if (cnt < 1) return;

	for (int i = 0; i < cnt; i++) {
		if (m_zoomScale != 1.0) {
			ppt[i].x = ROUND_INT(ppt[i].x * m_zoomScale);
			ppt[i].y = ROUND_INT(ppt[i].y * m_zoomScale);
		}
	}
}

void CZoomView::ScaledPointToPixelPoint(POINT* ppt, int cnt)
{
	if (cnt < 1) return;

	for (int i = 0; i < cnt; i++) {
		if (m_zoomScale != 1.0) {
			ppt[i].x = ROUND_INT(ppt[i].x / m_zoomScale);
			ppt[i].y = ROUND_INT(ppt[i].y / m_zoomScale);
		}
	}
}

void CZoomView::DevLog(POINT* ppt, int cnt)
{
	if (cnt < 1) return;

	CPoint pt;
	GetDeviceScrollOffset(pt);
	for (int i = 0; i < cnt; i++) {
		if (m_zoomScale != 1.0) {
			ppt[i].x = ROUND_INT(ppt[i].x * m_zoomScale);
			ppt[i].y = ROUND_INT(ppt[i].y * m_zoomScale);
		}
		ppt[i].x -= pt.x;
		ppt[i].y -= pt.y;
	}
}

int CZoomView::HitTest(CPoint point)		// point is DEVICE_POINT
{
	if ((m_rcSel.left != m_rcSel.right) || (m_rcSel.top != m_rcSel.bottom)) {
		CRect rcSel = m_rcSel;
		DevLog(rcSel);
		DevLog(point);

		CRect rc;

		// Top-Left
		rc.left = rcSel.left - _PH;
		rc.top = rcSel.top - _PH;
		rc.right = rc.left + _PS;
		rc.bottom = rc.top + _PS;
		if (rc.PtInRect(point))
			return htTopLeft;

		// Top-Right
		rc.left = rcSel.right - _PH;
		rc.top = rcSel.top - _PH;
		rc.right = rc.left + _PS;
		rc.bottom = rc.top + _PS;
		if (rc.PtInRect(point))
			return htTopRight;

		// Bottom-Left
		rc.left = rcSel.left - _PH;
		rc.top = rcSel.bottom - _PH;
		rc.right = rc.left + _PS;
		rc.bottom = rc.top + _PS;
		if (rc.PtInRect(point))
			return htBottomLeft;

		// Bottom-Right
		rc.left = rcSel.right - _PH;
		rc.top = rcSel.bottom - _PH;
		rc.right = rc.left + _PS;
		rc.bottom = rc.top + _PS;
		if (rc.PtInRect(point))
			return htBottomRight;

		// Top-Border
		rc.left = rcSel.left;
		rc.right = rcSel.right;
		rc.top = rcSel.top - _PH;
		rc.bottom = rc.top + _PS;
		if (rc.PtInRect(point))
			return htTop;

		// Bottom-Border
		rc.left = rcSel.left;
		rc.right = rcSel.right;
		rc.top = rcSel.bottom - _PH;
		rc.bottom = rc.top + _PS;
		if (rc.PtInRect(point))
			return htBottom;

		// Left-Border
		rc.left = rcSel.left - _PH;
		rc.right = rc.left + _PS;
		rc.top = rcSel.top;
		rc.bottom = rcSel.bottom;
		if (rc.PtInRect(point))
			return htLeft;

		// Right-Border
		rc.left = rcSel.right - _PH;
		rc.right = rc.left + _PS;
		rc.top = rcSel.top;
		rc.bottom = rcSel.bottom;
		if (rc.PtInRect(point))
			return htRight;

		if (rcSel.PtInRect(point))
			return htMiddle;
	}
	return htNothing;
}


LRESULT CZoomView::OnSetCursor(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND hwnd  = (HWND)wParam;
	UINT nHitTest = (UINT)LOWORD(lParam);
	if (nHitTest != HTCLIENT || hwnd != m_hWnd) {
		bHandled = FALSE;
		return 0;
	}

	if (m_zoomMode == ZoomIn)
		::SetCursor((GetKeyState(VK_SHIFT) < 0) ? m_hcZoomOut : m_hcZoomIn);
	else if (m_zoomMode == ZoomGrab)
		::SetCursor((GetKeyState(VK_CONTROL) < 0) ? ((GetKeyState(VK_SHIFT) < 0) ? m_hcZoomOut : m_hcZoomIn) : m_hcHandOpen);
	else if (m_zoomMode == ZoomSel) {
		CPoint point(GetMessagePos());
		ScreenToClient(&point);
		LogDev(point);
		switch (HitTest(point)) {
			case htTopLeft:
			case htBottomRight:	::SetCursor(m_hcTLtoBR); break;
			case htTopRight:
			case htBottomLeft:	::SetCursor(m_hcBLtoTR); break;
			case htTop:
			case htBottom:		::SetCursor(m_hcVert); break;
			case htLeft:
			case htRight:		::SetCursor(m_hcHorz); break;
			case htMiddle:		::SetCursor(m_hcMove); break;
			default:			::SetCursor(m_hcSel); break;
		}
	}
	else {
		bHandled = FALSE;
		return 0;
	}
	return TRUE;
}

LRESULT CZoomView::OnHandlerMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	UINT keyState = GET_KEYSTATE_WPARAM(wParam);
	if (keyState & (MK_CONTROL | MK_MBUTTON)) {
		short delta = GET_WHEEL_DELTA_WPARAM(wParam);
		double scale = m_zoomScale;
		if (delta > 0) {
			scale *= 1.1;
			if (scale > zoomFactor[numZoomFactor - 1])
				scale = zoomFactor[numZoomFactor - 1];
		}
		else {
			scale /= 1.1;
			if (scale < zoomFactor[0])
				scale = zoomFactor[0];
		}

		if (scale != m_zoomScale)
			DoZoomScale(NULL, scale);
	}
	else
		bHandled = FALSE;
	return 0;
}

int CZoomView::OnMouseActivate(HWND hDesktopWnd, UINT nHitTest, UINT message)
{
	bool bNoActive = (::GetActiveWindow() == NULL);
	if (GetFocus() != m_hWnd) {
		SetFocus();
		return (nHitTest == HTCLIENT && bNoActive) ? MA_ACTIVATEANDEAT : MA_ACTIVATE;
	}
	return MA_ACTIVATE;
}

static int __fastcall calc_gap(int pos, int gap)
{
	if (!gap) return pos;

	// グリッド吸着計算
	int s = pos / gap;
	int m = pos % gap;
	if (m)
		s += ((m < (gap / 2)) ? 0 : 1);
	return s * gap;
}

void CZoomView::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	DefWindowProc();

	bool vkControl = (GetKeyState(VK_CONTROL) < 0);
	bool vkShift = (GetKeyState(VK_SHIFT) < 0);

	// 最終ポイント（LOGICAL POINT)の保存
	m_ptLast = point;
	LogDev(point);

	if (m_zoomMode == ZoomGrab) {
		if (!vkControl && GetStyle() & (WS_VSCROLL | WS_HSCROLL)) {
			m_bCaptured = TRUE;
			SetCapture();
			::SetCursor(m_hcHandClose);
		}
	}
	else if (m_zoomMode == ZoomSel) {
		// Capture the mouse for zooming in
		m_bCaptured = TRUE;
		SetCapture();

		m_trackMode = trackAny;

		int hit = HitTest(point);

		if (vkControl || vkShift) {
			CSize gap = m_grid;
			if (vkShift) {
				gap.cx <<= 1;
				gap.cy <<= 1;
			}
			point.x = calc_gap(point.x, gap.cx);
			point.y = calc_gap(point.y, gap.cy);
		}

		if (hit == htTopLeft) {
			m_rcTrack.left = m_rcSel.right;
			m_rcTrack.top = m_rcSel.bottom;
			m_rcTrack.BottomRight() = point;
		}
		else if (hit == htTopRight) {
			m_rcTrack.left = m_rcSel.left;
			m_rcTrack.top = m_rcSel.bottom;
			m_rcTrack.BottomRight() = point;
		}
		else if (hit == htBottomLeft) {
			m_rcTrack.left = m_rcSel.right;
			m_rcTrack.top = m_rcSel.top;
			m_rcTrack.BottomRight() = point;
		}
		else if (hit == htBottomRight) {
			m_rcTrack.left = m_rcSel.left;
			m_rcTrack.top = m_rcSel.top;
			m_rcTrack.BottomRight() = point;
		}
		else if (hit == htTop) {
			m_rcTrack.left = m_rcSel.left;
			m_rcTrack.right = m_rcSel.right;
			m_rcTrack.top = m_rcSel.bottom;
			m_rcTrack.bottom = point.y;
			m_trackMode = trackVert;
		}
		else if (hit == htBottom) {
			m_rcTrack.left = m_rcSel.left;
			m_rcTrack.right = m_rcSel.right;
			m_rcTrack.top = m_rcSel.top;
			m_rcTrack.bottom = point.y;
			m_trackMode = trackVert;
		}
		else if (hit == htLeft) {
			m_rcTrack.left = m_rcSel.right;
			m_rcTrack.right = point.x;
			m_rcTrack.top = m_rcSel.top;
			m_rcTrack.bottom = m_rcSel.bottom;
			m_trackMode = trackHorz;
		}
		else if (hit == htRight) {
			m_rcTrack.left = m_rcSel.left;
			m_rcTrack.right = point.x;
			m_rcTrack.top = m_rcSel.top;
			m_rcTrack.bottom = m_rcSel.bottom;
			m_trackMode = trackHorz;
		}
		else if (hit == htMiddle) {
			m_ptTrack.x = point.x - m_rcSel.left;
			m_ptTrack.y = point.y - m_rcSel.top;
			m_trackMode = trackMove;
		}

		// erase old select
		CClientDC dc(m_hWnd);
		DrawSelect(dc, m_rcSel);
		if (hit == htNothing) {
			m_rcTrack.SetRect(point.x, point.y, point.x, point.y);
			m_rcSel.SetRectEmpty();
		}
		else
			DrawSelect(dc, m_rcSel);
	}
}

void CZoomView::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	DefWindowProc();

	LogDev(point);
	if (m_zoomMode == ZoomGrab) {
		if (m_bCaptured) {
			m_bCaptured = FALSE;
			ReleaseCapture();
			::SetCursor(m_hcHandOpen);
		}
		else if (GetKeyState(VK_CONTROL) < 0) {
			if (GetKeyState(VK_SHIFT) < 0)
				DoZoomOut(&point);
			else
				DoZoomIn(&point);
		}
	}
	else if (m_zoomMode == ZoomIn) {
		if (GetKeyState(VK_SHIFT) < 0)
			DoZoomOut(&point);
		else
			DoZoomIn(&point);
	}
	else if (m_zoomMode == ZoomSel && m_bCaptured) {
		m_bCaptured = FALSE;
		ReleaseCapture();
		if ((m_rcSel.left == m_rcSel.right) && (m_rcSel.top == m_rcSel.bottom))
			m_rcSel.SetRectEmpty();
		NotifySelectionChanged();
	}
}

void CZoomView::OnRButtonUp(UINT /*nFlags*/, CPoint point)
{
	DefWindowProc();
	LogDev(point);
	if (m_zoomMode == ZoomIn || (m_zoomMode == ZoomGrab && GetKeyState(VK_CONTROL) < 0)) {
		DoZoomOut(&point);
	}

}

void CZoomView::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	// Pass the message along
	DefWindowProc();

	CPoint ptLog = point;
	LogDev(point);
	NotifyPosChanged(point);

	if (!m_bCaptured) return;

	if (m_zoomMode == ZoomGrab) {
		int nSBCode;
		int delta;
		DWORD style = GetStyle();
		if (style & WS_HSCROLL && ptLog.x != m_ptLast.x) {
			delta = abs(m_ptLast.x - ptLog.x);
			nSBCode = (ptLog.x > m_ptLast.x) ? SB_LINEUP : SB_LINEDOWN;
			DoScroll(SB_HORZ, nSBCode, (int&)m_ptOffset.x, m_sizeAll.cx, m_sizePage.cx, delta);
		}
		if (style & WS_VSCROLL && point.y != m_ptLast.y) {
			delta = abs(m_ptLast.y - ptLog.y);
			nSBCode = (ptLog.y > m_ptLast.y) ? SB_LINEUP : SB_LINEDOWN;
			DoScroll(SB_VERT, nSBCode, (int&)m_ptOffset.y, m_sizeAll.cy, m_sizePage.cy, delta);
		}
	}
	else if (m_zoomMode == ZoomSel) {
		CSize gap(0, 0);
		if (GetKeyState(VK_CONTROL) < 0)
			gap = m_grid;
		else if (GetKeyState(VK_SHIFT) < 0) {
			gap = m_grid;
			gap.cx <<= 1;
			gap.cy <<= 1;
		}

		CRect rcSelOld = m_rcSel;
		CRect rcs(0, 0, m_sizeImage.cx, m_sizeImage.cy);
		CRect rcd, rcTo(0, 0, 0, 0);
		if (m_trackMode == trackMove) {
			rcd.left = point.x - m_ptTrack.x;
			rcd.top = point.y - m_ptTrack.y;

			// グリッド吸着計算
			rcd.left = calc_gap(rcd.left, gap.cx);
			rcd.top = calc_gap(rcd.top, gap.cy);
			rcd.right = rcd.left + m_rcSel.Width();
			rcd.bottom = rcd.top + m_rcSel.Height();

			if (rcd.left < 0)				rcd.OffsetRect(-rcd.left, 0);
			if (rcd.top < 0)				rcd.OffsetRect(0, -rcd.top);
			if (rcd.right > rcs.right)		rcd.OffsetRect(rcs.right - rcd.right, 0);
			if (rcd.bottom > rcs.bottom)	rcd.OffsetRect(0, rcs.bottom - rcd.bottom);

			if (rcd.left < m_rcSel.left)	rcTo.left = rcd.left - m_rcSel.left;
			else							rcTo.right = rcd.left - m_rcSel.left;

			if (rcd.top < m_rcSel.top)		rcTo.top = rcd.top - m_rcSel.top;
			else							rcTo.bottom = rcd.top - m_rcSel.top;

			m_rcSel = rcd;
		}
		else {
			// グリッド吸着計算
			point.x = calc_gap(point.x, gap.cx);
			point.y = calc_gap(point.y, gap.cy);

			// Draw new rect
			if (m_trackMode == trackVert)		m_rcTrack.bottom = point.y;
			else if (m_trackMode == trackHorz)	m_rcTrack.right = point.x;
			else								m_rcTrack.BottomRight() = point;

			rcd = m_rcTrack;
			rcd.NormalizeRect();
			m_rcSel = rcs & rcd;

			if (m_rcSel.Width()) {
				if (rcSelOld.left != m_rcSel.left)
					rcTo.left = m_rcSel.left - rcSelOld.left;
				else if (rcSelOld.right != m_rcSel.right)
					rcTo.right = m_rcSel.right - rcSelOld.right;
			}

			if (m_rcSel.Height()) {
				if (rcSelOld.top != m_rcSel.top)
					rcTo.top = m_rcSel.top - rcSelOld.top;
				else if (rcSelOld.bottom != m_rcSel.bottom)
					rcTo.bottom = m_rcSel.bottom - rcSelOld.bottom;
			}
		}

		if (rcSelOld != m_rcSel) {
			CClientDC dc(m_hWnd);
			DrawSelect(dc, rcSelOld);
			DrawSelect(dc, m_rcSel);
			NotifySelectionChanged();
		}

		// スクロール
		if (!rcTo.IsRectNull()) {
			rcs.SetRect(0, 0, m_sizeClient.cx, m_sizeClient.cy);
			rcd = m_rcSel;
			DevLog(rcd);

			CPoint pos(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
			if (rcTo.left < 0 && rcd.left < rcs.left)
				pos.x -= rcs.left - rcd.left;
			else if (rcTo.left > 0 && rcd.left > rcs.right)
				pos.x += rcd.left - rcs.right;
			else if (rcTo.right < 0 && rcd.right < rcs.left)
				pos.x -= rcs.left - rcd.right;
			else if (rcTo.right > 0 && rcd.right > rcs.right)
				pos.x += rcd.right - rcs.right;

			if (rcTo.top < 0 && rcd.top < rcs.top)
				pos.y -= rcs.top - rcd.top;
			else if (rcTo.top > 0 && rcd.top > rcs.bottom)
				pos.y += rcd.top - rcs.bottom;
			else if (rcTo.bottom < 0 && rcd.bottom < rcs.top)
				pos.y -= rcs.top - rcd.bottom;
			else if (rcTo.bottom > 0 && rcd.bottom > rcs.bottom)
				pos.y += rcd.bottom - rcs.bottom;

			ScrollToPosition(pos);
		}
	}

	// 最終ポイントの更新
	m_ptLast = ptLog;

}

void CZoomView::NotifyScaleChanged()
{
	ZOOM_SCALE zs;
	zs.hdr.hwndFrom = m_hWnd;
	zs.hdr.idFrom = GetDlgCtrlID();
	zs.hdr.code = ZMVN_SCALECHANGED;
	zs.scale = m_zoomScale;
	GetParent().SendMessage(WM_NOTIFY, (WPARAM)zs.hdr.idFrom, (LPARAM)&zs);
}

void CZoomView::NotifySelectionChanged()
{
	ZOOM_CLIP zc;
	zc.hdr.hwndFrom = m_hWnd;
	zc.hdr.idFrom = GetDlgCtrlID();
	zc.hdr.code = ZMVN_CLIPCHANGED;
	zc.rcClip = m_rcSel;
	GetParent().SendMessage(WM_NOTIFY, (WPARAM)zc.hdr.idFrom, (LPARAM)&zc);
}

void CZoomView::NotifyPosChanged(CPoint& pt)		// point is device
{
	if (!m_dib.isNull()) {
		ZOOM_MOUSEPOINT zm;
		memset(&zm, 0, sizeof(zm));
		zm.hdr.hwndFrom = m_hWnd;
		zm.hdr.idFrom = GetDlgCtrlID();
		zm.hdr.code = ZMVN_MOUSEPOINT;

		// イメージ上のマウスカーソル位置
		zm.pt = pt;
		// 選択矩形
		zm.rcClip = m_rcSel;
		// ドラッグ中か否か
		zm.bInDrag = (m_bCaptured && m_zoomMode == ZoomSel);

		GetParent().SendMessage(WM_NOTIFY, (WPARAM)zm.hdr.idFrom, (LPARAM)&zm);
	}
}

void CZoomView::NotifyVisibleChanged()
{
	if (!m_dib.isNull()) {
		ZOOM_VISIBLEINFO zv;
		memset(&zv, 0, sizeof(zv));
		zv.hdr.hwndFrom = m_hWnd;
		zv.hdr.idFrom = GetDlgCtrlID();
		zv.hdr.code = ZMVN_VISIBLECHANGED;

		// イメージ全体サイズ
		zv.sizeImage = m_sizeImage;

		// 表示しているイメージの矩形
		CRect rc;
		GetClientRect(&rc);
		LogDev(rc);
		if (rc.Width() > m_sizeImage.cx) {
			rc.left = 0;
			rc.right = m_sizeImage.cx;
		}
		else if (rc.right > m_sizeImage.cx)
			rc.right = m_sizeImage.cx;

		if (rc.Height() > m_sizeImage.cy) {
			rc.top = 0;
			rc.bottom = m_sizeImage.cy;
		}
		else if (rc.bottom > m_sizeImage.cy)
			rc.bottom = m_sizeImage.cy;

		zv.rectVisible = rc;

		GetParent().SendMessage(WM_NOTIFY, (WPARAM)zv.hdr.idFrom, (LPARAM)&zv);
	}
}

CPoint CZoomView::GetCenterPoint(void)
{
	CRect rect;
	GetClientRect(&rect);
	return CPoint(rect.Width() / 2, rect.Height() / 2);
}

void CZoomView::GetScrollLimit(CPoint& ptLim)
{
	ptLim.x = m_sizeAll.cx - m_sizeClient.cx - 1;
	ptLim.x = max(0, ptLim.x);
	ptLim.y = m_sizeAll.cy - m_sizeClient.cy - 1;
	ptLim.y = max(0, ptLim.y);
}

void CZoomView::CenterOnPoint(CPoint ptCenter) // center in logical coords
{
	CRect rect;
	GetClientRect(&rect);

	CPoint pt = ptCenter;
	pt.x -= rect.Width() / 2;
	pt.y -= rect.Height() / 2;

	DWORD style = GetStyle();
	if ((style & WS_HSCROLL) == 0) pt.x = 0;
	if ((style & WS_VSCROLL) == 0) pt.y = 0;

	ScrollToPosition(pt);
}

void CZoomView::ScrollToPosition(CPoint pt)			// point is logical coordinate
{
	CPoint ptLim;
	GetScrollLimit(ptLim);

	pt.x = max(0, min(pt.x, ptLim.x));
	pt.y = max(0, min(pt.y, ptLim.y));

	CPoint ptOffset;
	GetScrollOffset(ptOffset);
	if (pt != ptOffset) {
		int xScroll = ptOffset.x - pt.x;
		int yScroll = ptOffset.y - pt.y;
		SetScrollOffset(pt, TRUE);
		ScrollWindowEx(xScroll, yScroll, m_uScrollFlags);
	}

	NotifyVisibleChanged();
}

LRESULT CZoomView::OnHandlerSetDrawMode(WPARAM wParam, LPARAM lParam)
{
	int oldMode = m_nDrawingMode;
	if ((int)wParam == m_nDrawingMode) return (LRESULT)m_nDrawingMode;

	m_nDrawingMode = (int)wParam;

	ClearCachedImage();				// HBITMAP, Bitmap* の削除とクリア

	if (!m_dib.isNull()) {
		BITMAPINFO* pbi = m_dib.BitmapInfo();
		if (!wParam) {
			CClientDC dc(nullptr);
			m_hbm = m_dib.GetHBitmap(dc);
		}
		else
			m_gdipBitmap = new Bitmap(m_dib.BitmapInfo(), m_dib.FindBits());
		Invalidate(FALSE);
	}
	return (LRESULT)oldMode;
}

void CZoomView::OnHandlerCenterPosition(WPARAM wParam, LPARAM lParam)
{
	CPoint ptCenter(m_sizeAll.cx / 2, m_sizeAll.cy / 2);
	DevLog(ptCenter);
	CenterOnPoint(ptCenter);
}

LRESULT CZoomView::OnHandlerZoomIn(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case DOZOOM_IN:			DoZoomIn();			break;
	case DOZOOM_OUT:		DoZoomOut();		break;
	case DOZOOM_FULL:		DoZoomFitClient();	break;
	case DOZOOM_DEFAULT:	DoZoomDefault();	break;
	case DOZOOM_FITHORZ:	DoZoomFitHorz();	break;
	case DOZOOM_FITVERT:	DoZoomFitVert();	break;
	default:				break;
	}
	return 0;
}

void CZoomView::OnHandlerScaleMenu(WPARAM wParam, LPARAM lParam)
{
	CPoint pt = *(POINT*)lParam;
	ScaleMenu(pt);
}

void CZoomView::OnHandlerSetGrid(WPARAM wParam, LPARAM lParam)
{
	// wParam: TRUE=show_grid, FALSE=hide_grid
	// lParam: 0>lParam=LOWORD(x_grid_size), HIWORD(y_grid_size)

	DWORD style = GetStyle() & ZMVS_GRID;
	DWORD styleNew = (wParam != 0) ? ZMVS_GRID : 0;

	BOOL bRedraw = (style != styleNew);
	CSize gridNew(LOWORD(lParam), HIWORD(lParam));
	if (m_grid != gridNew && gridNew.cx && gridNew.cy) {
		m_grid = gridNew;
		if (styleNew == ZMVS_GRID)
			bRedraw = TRUE;
	}
	ModifyStyle(ZMVS_GRID, styleNew);

	if (bRedraw)
		Invalidate(TRUE);
}

LRESULT CZoomView::OnHandlerCanZoom(WPARAM wParam, LPARAM lParam)
{
	BOOL bZoomIn = m_dib.isNull() ? FALSE : (m_zoomScale < zoomFactor[numZoomFactor - 1]);
	BOOL bZoomOut = m_dib.isNull() ? FALSE : (m_zoomScale > zoomFactor[0]);
	return (LRESULT)(MAKELONG(bZoomIn, bZoomOut));
}

BOOL CZoomView::DoZoomScale(POINT* ppt, double scale)	// point in device coordinates
{
	if (m_dib.isNull() || scale < zoomFactor[0] || scale > zoomFactor[numZoomFactor - 1] || scale == m_zoomScale) return FALSE;

	// Set the current center point.
	CPoint pt;
	if (ppt) {
		pt = *ppt;
	}
	else {
		pt = GetCenterPoint();
		LogDev(pt);
	}

	m_zoomScale = scale;
	SetScrollSize(ROUND_INT(m_sizeImage.cx * m_zoomScale), ROUND_INT(m_sizeImage.cy * m_zoomScale), TRUE);

	// スケーリング起点
	PixelPointToScaledPoint(pt);
	CenterOnPoint(pt);

	NotifyScaleChanged();
	return TRUE;
}

BOOL CZoomView::DoZoomIn(POINT* ppt)   // point in device coordinates
{
	int i;
	for (i = numZoomFactor - 1; i >= 0; i--) {
		if (zoomFactor[i] <= m_zoomScale)
			break;
	}
	return DoZoomScale(ppt, zoomFactor[min(numZoomFactor - 1, max(0, i + 1))]);
}

BOOL CZoomView::DoZoomOut(POINT* ppt)   // point in device coordinates
{
	int i;
	for (i = 0; i < numZoomFactor; i++) {
		if (zoomFactor[i] >= m_zoomScale)
			break;
	}
	return DoZoomScale(ppt, zoomFactor[min(numZoomFactor - 1, max(0, i - 1))]);
}

BOOL CZoomView::DoZoomDefault(POINT* point)   // point in device coordinates
{
	return DoZoomScale(point, zoomFactor[ZOOM_DEFAULT_INDEX]);
}

BOOL CZoomView::DoZoomFitClient()
{
	if (m_dib.isNull()) return FALSE;

	CRect rc(0, 0, m_sizeClient.cx, m_sizeClient.cy);
	DWORD style = GetStyle();
	if (style & WS_HSCROLL) rc.bottom += GetSystemMetrics(SM_CYHSCROLL);
	if (style & WS_VSCROLL) rc.right += GetSystemMetrics(SM_CXVSCROLL);

	// クライアント矩形いっぱいではなく、若干のマージンを確保する
	if (rc.Width() > INNER_MARGIN * 2) rc.InflateRect(-INNER_MARGIN, 0);
	if (rc.Height() > INNER_MARGIN * 2) rc.InflateRect(0, -INNER_MARGIN);

	double xscale = (double)rc.Width() / (double)m_sizeImage.cx;
	double yscale = (double)rc.Height() / (double)m_sizeImage.cy;
	double scale = MIN(xscale, yscale);
	scale = MAX(zoomFactor[0], MIN(scale, zoomFactor[numZoomFactor - 1]));
	if (scale != m_zoomScale)
		return DoZoomScale(NULL, scale);
	return FALSE;
}

BOOL CZoomView::DoZoomFitHorz()
{
	if (m_dib.isNull()) return FALSE;

	CRect rc(0, 0, m_sizeClient.cx, m_sizeClient.cy);
	DWORD style = GetStyle();
	if (style & WS_HSCROLL) rc.bottom += GetSystemMetrics(SM_CYHSCROLL);
	if (style & WS_VSCROLL) rc.right += GetSystemMetrics(SM_CXVSCROLL);

	double scale = (double)rc.Width() / (double)m_sizeImage.cx;
	if (scale * m_sizeImage.cy > (double)rc.Height())
		scale = (double)(rc.Width() - GetSystemMetrics(SM_CXVSCROLL)) / (double)m_sizeImage.cx;
	scale = MAX(zoomFactor[0], MIN(scale, zoomFactor[numZoomFactor - 1]));

	if (scale != m_zoomScale)
		return DoZoomScale(NULL, scale);
	return FALSE;
}

BOOL CZoomView::DoZoomFitVert()
{
	if (m_dib.isNull()) return FALSE;

	CRect rc(0, 0, m_sizeClient.cx, m_sizeClient.cy);
	DWORD style = GetStyle();
	if (style & WS_HSCROLL) rc.bottom += GetSystemMetrics(SM_CYHSCROLL);
	if (style & WS_VSCROLL) rc.right += GetSystemMetrics(SM_CXVSCROLL);

	double scale = (double)rc.Height() / (double)m_sizeImage.cy;
	if (scale * m_sizeImage.cx > (double)rc.Width())
		scale = (double)(rc.Height() - GetSystemMetrics(SM_CYHSCROLL)) / (double)m_sizeImage.cy;
	scale = MAX(zoomFactor[0], MIN(scale, zoomFactor[numZoomFactor - 1]));

	if (scale != m_zoomScale)
		return DoZoomScale(NULL, scale);
	return FALSE;
}

void CZoomView::SetZoomMode(ZoomMode_ zoomMode)
{
	if (zoomMode >= ZOOMMODE_NONE && zoomMode <= ZOOMMODE_SEL && zoomMode != m_zoomMode) {
		m_zoomMode = zoomMode;
		// Force cursor change now
		BOOL b;
		OnSetCursor(WM_SETCURSOR, NULL, MAKELONG(HTCLIENT, 0), b);
	}
}

double CZoomView::GetZoomScale()
{
	return m_zoomScale;
}

#define ID_SCALE_START		0x6000

void CZoomView::ScaleMenu(CPoint pt)	// pt is screen
{
	CMenu pop;
	pop.CreatePopupMenu();

	TCHAR buf[MAX_PATH];
	for (int i = 0; i < numZoomFactor; i++) {
		int iScale = (int)(zoomFactor[i] * 1000);
		int s = iScale / 10;
		int m = iScale % 10;
		if (!m)
			wsprintf(buf, _T("%d%%"), s);
		else
			wsprintf(buf, _T("%d.%d%%"), s, m);

		UINT flg = MF_STRING;
		if (zoomFactor[i] == m_zoomScale)
			flg |= MF_CHECKED;

		pop.AppendMenu(flg, ID_SCALE_START + i, buf);
	}
	int id = pop.TrackPopupMenu(TPM_RETURNCMD, pt.x, pt.y, m_hWnd);
	if (id > 0) DoZoomScale(NULL, zoomFactor[id - ID_SCALE_START]);
}

BOOL CZoomView::GetClipRect(LPRECT prc)
{
	SetRectEmpty(prc);
	if (m_rcSel.left || m_rcSel.right || m_rcSel.top || m_rcSel.bottom) {
		::CopyRect(prc, &m_rcSel);
		return TRUE;
	}
	return FALSE;
}

void CZoomView::SetClipRect(LPRECT prc)
{
	if (::IsWindow(m_hWnd)) {
		CClientDC dc(m_hWnd);
		if (m_rcSel.left || m_rcSel.right || m_rcSel.top || m_rcSel.bottom)
			DrawSelect(dc, m_rcSel);

		m_rcSel = *prc;
		if (m_rcSel.left || m_rcSel.right || m_rcSel.top || m_rcSel.bottom)
			DrawSelect(dc, m_rcSel);

		NotifySelectionChanged();
	}
}

int CZoomView::GetScaleFactorList(int* ppScale, int numMax)
{
	numMax = min(numMax, numZoomFactor);
	for (int i = 0; i < numMax; i++)
		*ppScale++ = ROUND_INT(zoomFactor[i] * 1000);
	return numMax;
}

BOOL CZoomView::SetZoomViewInfo(ZOOMVIEWINFO* pzi)
{
	if (!pzi) return FALSE;

	DWORD style = GetStyle();
	BOOL bRedraw = FALSE;
	if (pzi->fMask & ZIF_SHOWGRID) {
		DWORD style = GetStyle();
		DWORD styleNew = style;
		if (pzi->showGrid)
			styleNew |= ZMVS_GRID;
		else
			styleNew &= ~ZMVS_GRID;

		if (style != styleNew) {
			SetWindowLongPtr(GWL_STYLE, styleNew);
			style = styleNew;
			bRedraw = TRUE;
		}
	}
	if (pzi->fMask & ZIF_GRID) {
		CSize grid(pzi->grid);
		if (m_grid != grid && grid.cx > 0 && grid.cy > 0) {
			m_grid = grid;
			if (style & ZMVS_GRID)
				bRedraw = TRUE;
		}
	}
	if (pzi->fMask & ZIF_GRIDCOLOR) {
		if (m_crGrid != pzi->crGrid) {
			m_crGrid = pzi->crGrid;
			if (style & ZMVS_GRID)
				bRedraw = TRUE;
		}
	}
	if (pzi->fMask & ZIF_MODE)
		SetZoomMode((ZoomMode_)pzi->mode);
	if (pzi->fMask & ZIF_CLIP) {
		CRect rc(&pzi->rcClip);
		if (rc != m_rcSel) {
			m_rcSel = rc;
			bRedraw = TRUE;
		}
	}
	if (pzi->fMask & ZIF_BKCOLOR) {
		if (pzi->crBack != m_crBkgnd) {
			SetBkColor(FALSE, pzi->crBack);
			bRedraw = TRUE;
		}
	}
	if (pzi->fMask & ZIF_SCALE) {
		double scale = (double)pzi->scale / 1000;
		if (scale != m_zoomScale) {
			DoZoomScale(NULL, scale);
			bRedraw = FALSE;
		}
	}
	if (pzi->cbSize > sizeof(ZOOMVIEWINFO_V1)) {
		if (pzi->fMask & ZIF_CENTERPOSITION) {
			CPoint pt = pzi->ptCenterPosition;
			PixelPointToScaledPoint(pt);
			CenterOnPoint(pt);
		}
	}

	if (bRedraw)
		Invalidate(TRUE);
	return TRUE;
}

BOOL CZoomView::GetZoomViewInfo(ZOOMVIEWINFO* pzi)
{
	if (!pzi)
		return FALSE;

	if (pzi->fMask & ZIF_GRID)		pzi->grid = m_grid;
	if (pzi->fMask & ZIF_GRIDCOLOR)	pzi->crGrid = m_crGrid;
	if (pzi->fMask & ZIF_MODE)		pzi->mode = (int)m_zoomMode;
	if (pzi->fMask & ZIF_CLIP)		CopyRect(&pzi->rcClip, m_rcSel);
	if (pzi->fMask & ZIF_SCALE)		pzi->scale = (int)(m_zoomScale * 1000);
//	if (pzi->fMask & ZIF_BITMAP)	pzi->hbm = m_dib;
	if (pzi->fMask & ZIF_BKCOLOR)	pzi->crBack = m_crBkgnd;
	if (pzi->fMask & ZIF_SHOWGRID)	pzi->showGrid = (GetStyle() & ZMVS_GRID) ? TRUE : FALSE;

	if (pzi->cbSize > sizeof(ZOOMVIEWINFO_V1)) {
		if (pzi->fMask & ZIF_CENTERPOSITION) {
			CPoint pt = GetCenterPoint();
			LogDev(pt);
			pzi->ptCenterPosition = pt;
		}
	}
	return TRUE;
}

BOOL CZoomView::TrackImage(HIMAGELIST hImage, LPPOINT ppt)
{
	Invalidate(FALSE);
	UpdateWindow();

	CRect rcw;
	GetWindowRect(&rcw);
	CRect rcc(0, 0, m_sizeClient.cx, m_sizeClient.cy);
	ClientToScreen(&rcc);
	int xdelta = rcc.left - rcw.left;
	int ydelta = rcc.top - rcw.top;

	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	ImageList_BeginDrag(hImage, 0, 0, 0);

	CPoint ptDrag = pt;
	ptDrag.x += xdelta;
	ptDrag.y += ydelta;
	ImageList_DragEnter(m_hWnd, ptDrag.x, ptDrag.y);

	SetCapture();
	HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_CROSS));
	BOOL bOK = FALSE;
	for (;;) {
		MSG msg;
		WaitMessage();
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			BOOL bProcEnter = FALSE;
			if (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) {
				pt = CPoint(LOWORD(msg.lParam), HIWORD(msg.lParam));
				bProcEnter = TRUE;
			}
			else if (msg.message == WM_KEYDOWN) {
				bProcEnter = TRUE;
				switch (msg.wParam) {
					case VK_ESCAPE:
					case VK_RETURN:
						pt = msg.pt;
						ScreenToClient(&pt);
						break;

					default:
						break;
				}
			}

			if (!bProcEnter) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				continue;
			}

			ptDrag = pt;
			ptDrag.x += xdelta;
			ptDrag.y += ydelta;
			ImageList_DragMove(ptDrag.x, ptDrag.y);

			LogDev(pt);
			NotifyPosChanged(pt);

			if (msg.message == WM_RBUTTONDOWN ||
				(msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE))
				break;
			else if (msg.message == WM_LBUTTONDOWN ||
				(msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN)) {
				if (SendMessage(WM_NCHITTEST, 0, MAKELPARAM(msg.pt.x, msg.pt.y)) == HTCLIENT)
					bOK = TRUE;
				break;
			}
		}
		else
			continue;
	}
	ImageList_DragLeave(m_hWnd);
	ImageList_EndDrag();

	SetCursor(hcur);
	ReleaseCapture();

	if (bOK) {
		ppt->x = pt.x;
		ppt->y = pt.y;
		return TRUE;
	}
	return FALSE;
}

BOOL CZoomView::ClientToDevice(LPPOINT ppt, int cnt)
{
	if (ppt && cnt > 0) {
		LogDev(ppt, cnt);
		return TRUE;
	}
	return FALSE;
}

BOOL CZoomView::DeviceToClient(LPPOINT ppt, int cnt)
{
	if (ppt && cnt > 0) {
		DevLog(ppt, cnt);
		return TRUE;
	}
	return FALSE;
}

COLORREF CZoomView::GetPixelColor(int x, int y)
{
	COLORREF clr = INVALID_COLOR;
	if (!m_dib.isNull()) {
		BITMAPINFOHEADER* pbh = (BITMAPINFOHEADER*)m_dib.BitmapInfo();
		if (pbh->biBitCount == 24 && 0 <= x && x < pbh->biWidth && 0 <= y && y <= pbh->biHeight) {
			int widthbytes = WIDTHBYTES(pbh->biBitCount * pbh->biWidth);
			BYTE* pb = (BYTE*)m_dib.FindBits() + widthbytes * (pbh->biHeight - y) + x;
			clr = RGB(*(pb + 2), *(pb + 1), *pb);
		}
	}
	return clr;
}

///////////////////////////////////////////////////////////////

//

extern "C"
LRESULT CALLBACK ZoomViewWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CZoomView* pWnd = new CZoomView;
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}
