//////////////////////////////////////////////////////////////////////
// preview.cpp


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "stdafx.h"
#include <limits.h>
#include "lsimg.h"
#include "dib_sup.h"
#include "resource.h"
#include "preview.h"

extern HINSTANCE ghInst;

const SIZE CPreView::sizeDefault = {0,0};

/////////////////////////////////////////////////////////////////////////////
// CPreView construction/destruction

// Special mapping modes just for CPreView implementation
#define MM_NONE             0
	// standard GDI mapping modes are > 0

LRESULT CALLBACK PreViewWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CPreView* pView;
	switch (msg)
	{
		case WM_NCCREATE:
			pView = new CPreView();
			pView->m_hWnd = hwnd;
			pView->m_hParent = ((LPCREATESTRUCT)lParam)->hwndParent;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pView);
			return TRUE;

		case WM_NCDESTROY:
			pView = (CPreView*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
			delete pView;
			return DefWindowProc(hwnd, msg, wParam, lParam);

		case WM_GETDLGCODE:
			return DLGC_STATIC;
	}

	pView = (CPreView*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (pView)
		return pView->WndProc(hwnd, msg, wParam, lParam);
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


CPreView::CPreView()
{
	m_nMapMode = MM_NONE;
	m_hWnd = NULL;
	m_hParent = NULL;
	m_totalLog = CSize(0, 0);
	m_totalDev = CSize(0, 0);
	m_pageDev = CSize(0, 0);
	m_lineDev = CSize(0, 0);
	m_origTotalDev = CSize(0, 0);
	m_origPageDev = CSize(0, 0);
	m_origLineDev = CSize(0, 0);
	m_bCaptured = FALSE;
	m_bCenter = TRUE;
	m_bInsideUpdate = FALSE;
	m_bHScroll = m_bVScroll = FALSE;
	memset(&m_vscroll, 0, sizeof(SCROLLINFO));
	memset(&m_hscroll, 0, sizeof(SCROLLINFO));
	m_image = m_image_s = NULL;
	m_hbm = m_hbmEffect = m_hbmBack = NULL;
	m_hpal = m_hpalEffect = NULL;
	m_bWhole = FALSE;

	// Load the zoom cursor
	m_hcHandOpen = ::LoadCursor(ghInst, MAKEINTRESOURCE(IDC_HANDOPEN));
	m_hcHandClose = ::LoadCursor(ghInst, MAKEINTRESOURCE(IDC_HANDCLOSE));

	m_crBkgnd = GetSysColor(COLOR_WINDOW);
}

CPreView::~CPreView()
{
	if (m_hcHandOpen)
		DestroyCursor(m_hcHandOpen);
	if (m_hcHandClose)
		DestroyCursor(m_hcHandClose);
	if (m_hbm)
		DeleteBitmap(m_hbm);
	if (m_hpal)
		DeletePalette(m_hpal);
	if (m_hbmEffect)
		DeleteBitmap(m_hbmEffect);
	if (m_hpalEffect)
		DeletePalette(m_hpalEffect);
	if (m_hbmBack)
		DeleteBitmap(m_hbmBack);
	if (m_image_s)
		delete m_image_s;
	if (m_image)
		delete m_image;
}

LRESULT CPreView::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	m_msg.hwnd = hwnd;
	m_msg.message = msg;
	m_msg.wParam = wParam;
	m_msg.lParam = lParam;
	m_msg.time = GetMessageTime();
	m_msg.pt = CPoint(GetMessagePos());

	switch (msg)
	{
		case WM_CREATE:
			return OnCreate((LPCREATESTRUCT)lParam);

		case WM_SETCURSOR:
			if (OnSetCursor((HWND)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam)))
				return TRUE;
			break;

		case WM_SIZE:
			OnSize((UINT)wParam, (int)LOWORD(lParam), (int)HIWORD(lParam));
			return 0;

		case WM_ERASEBKGND:
			return OnEraseBkgnd((HDC)wParam);

		case WM_PAINT:
			OnPaint();
			return 0;

		case WM_PALETTECHANGED:
			OnPaletteChanged((HWND)wParam);
			return 0;

		case WM_QUERYNEWPALETTE:
			return OnQueryNewPalette();

		case WM_LBUTTONDOWN:
			OnLButtonDown((UINT)wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
			return 0;

		case WM_LBUTTONUP:
			OnLButtonUp((UINT)wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
			return 0;

		case WM_MOUSEMOVE:
			OnMouseMove((UINT)wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
			return 0;

		case IVM_SETDIB:
			return (LRESULT)OnSetDIB((HDIB)lParam);

		case IVM_UPDATEEFFECT:
			NotifyUpdateEffect();
			if (wParam)
				RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
			return 0;

		case IVM_SETWHOLEVIEW:
			OnSetWholeView((BOOL)wParam);
			return 0;

		default:
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CPreView::Default()
{
	return DefWindowProc(m_msg.hwnd, m_msg.message, m_msg.wParam, m_msg.lParam);
}

/////////////////////////////////////////////////////////////////////////////
// CPreView

void CPreView::OnInitialUpdate()
{
	SetZoomSizes(CSize(0, 0));
}

void CPreView::OnPrepareDC(HDC hdc)
{
	SetMapMode(hdc, m_nMapMode);
	SetWindowExtEx(hdc, m_totalLog.cx, m_totalLog.cy, NULL);   // in logical coordinates
	SetViewportExtEx(hdc, m_totalDev.cx, m_totalDev.cy, NULL); // in device  coordinates

	// by default shift viewport origin in negative direction of scroll
	CPoint ptVpOrg = -GetDeviceScrollPosition();

	// Center full fit
	if (m_bCenter)
	{
		CRect rect;
		GetClientRect(m_hWnd, &rect);

		// if client area is larger than total device size,
		// override scroll positions to place origin such that
		// output is centered in the window
		if (m_totalDev.cx < rect.Width())
			ptVpOrg.x = (rect.Width() - m_totalDev.cx) / 2;
		if (m_totalDev.cy < rect.Height())
			ptVpOrg.y = (rect.Height() - m_totalDev.cy) / 2;
	}

	// Set the new origin
	SetViewportOrgEx(hdc, ptVpOrg.x, ptVpOrg.y, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// Set mode and scaling/scrolling sizes

int CPreView::GetScrollPos(int nBar) const
{
	return (nBar==SB_VERT) ? m_vscroll.nPos : m_hscroll.nPos;
}

int CPreView::SetScrollPos(int nBar, int nPos)
{
	int tmp;
	if (nBar==SB_VERT)
	{
		tmp = m_vscroll.nPos;
		m_vscroll.nPos = min(m_vscroll.nMax, max(nPos, m_vscroll.nMin));
	}
	else
	{
		tmp = m_hscroll.nPos;
		m_hscroll.nPos = min(m_hscroll.nMax, max(nPos, m_hscroll.nMin));
	}
	return tmp;
}

BOOL CPreView::GetScrollInfo(int nBar, SCROLLINFO* pInfo, UINT /*nMask*/)
{
	if (pInfo->fMask & SIF_RANGE)
		GetScrollRange(nBar, pInfo->nMin, pInfo->nMax);
	if (pInfo->fMask & SIF_PAGE)
		pInfo->nPage = (nBar==SB_VERT) ? m_vscroll.nPage : m_hscroll.nPage;
	if (pInfo->fMask & SIF_POS)
		pInfo->nPos = GetScrollPos(nBar);
	return TRUE;
}

BOOL CPreView::SetScrollInfo(int nBar, SCROLLINFO* pInfo)
{
	if (pInfo->fMask & SIF_RANGE)
		SetScrollRange(nBar, pInfo->nMin, pInfo->nMax);
	if (pInfo->fMask & SIF_PAGE)
	{
		if (nBar==SB_VERT)
			m_vscroll.nPage = pInfo->nPage;
		else
			m_hscroll.nPage = pInfo->nPage;
	}
	if (pInfo->fMask & SIF_POS)
		SetScrollPos(nBar, pInfo->nPos);
	return TRUE;
}


BOOL CPreView::GetScrollRange(int fnBar, int& nMin, int& nMax)
{
	nMin = (fnBar==SB_VERT) ? m_vscroll.nMin : m_hscroll.nMin;
	nMax = (fnBar==SB_VERT) ? m_vscroll.nMax : m_hscroll.nMax;
	return TRUE;
}

BOOL CPreView::SetScrollRange(int fnBar, int nMin, int nMax)
{
	if (fnBar==SB_VERT)
	{
		m_vscroll.nMin = nMin;
		m_vscroll.nMax = nMax;
		m_vscroll.nPos = min(m_vscroll.nMax, max(m_vscroll.nPos, m_vscroll.nMin));
	}
	else
	{
		m_hscroll.nMin = nMin;
		m_hscroll.nMax = nMax;
		m_hscroll.nPos = min(m_hscroll.nMax, max(m_hscroll.nPos, m_hscroll.nMin));
	}
	return TRUE;
}

int CPreView::GetScrollLimit(int nBar)
{
	int nMin, nMax;
	GetScrollRange(nBar, nMin, nMax);
	SCROLLINFO info;
	info.cbSize = sizeof(info);
	info.fMask = SIF_PAGE;
	if (GetScrollInfo(nBar, &info))
	{
		nMax -= __max(info.nPage - 1, 0);
	}
	return nMax;
}

void CPreView::CalcBars (void)
{
	{	// BLOCK for DC
		HDC hdc = GetWindowDC(NULL);
		SetMapMode(hdc, m_nMapMode);

		// Calculate new device units for scrollbar
		// Start with origional logical units from SetScrollPos
		m_pageDev = m_origPageDev;
		LPtoDP(hdc, (LPPOINT)&m_pageDev, 1);
		m_lineDev = m_origLineDev;
		LPtoDP(hdc, (LPPOINT)&m_lineDev, 1);

		ReleaseDC(NULL, hdc);
	}

	// Make sure of the range
	if (m_pageDev.cy < 0)  m_pageDev.cy = -m_pageDev.cy;
	if (m_lineDev.cy < 0)  m_lineDev.cy = -m_lineDev.cy;

	// If none specified - use one tenth
	if (m_pageDev.cx == 0) m_pageDev.cx = m_totalDev.cx / 10;
	if (m_pageDev.cy == 0) m_pageDev.cy = m_totalDev.cy / 10;
	if (m_lineDev.cx == 0) m_lineDev.cx = m_pageDev.cx  / 10;
	if (m_lineDev.cy == 0) m_lineDev.cy = m_pageDev.cy  / 10;

	// Now update the scrollbars
	if (m_hWnd != NULL) {
		UpdateBars();
		Invalidate(TRUE); // Zoom scale changed, redraw all
	}
}

void CPreView::SetZoomSizes (
	SIZE sizeTotal,
	const SIZE& sizePage, // in logical units
	const SIZE& sizeLine) // in logical units
{
	// Set up the defaults
	m_nMapMode = MM_ANISOTROPIC;		// Need for arbitrary scaling
	m_totalLog = sizeTotal;

	// Setup default Viewport extent to be conversion of Window extent
	// into device units.
	// BLOCK for DC
	{
		HDC hdc = GetWindowDC(NULL);
		SetMapMode(hdc, m_nMapMode);

		// total size
		m_origTotalDev = m_totalLog;
		LPtoDP(hdc, (LPPOINT)&m_origTotalDev, 1);
		ReleaseDC(NULL, hdc);
	}

	// Save the origional Viewport Extent
	m_totalDev.cx = m_origTotalDev.cx;
	m_totalDev.cy = m_origTotalDev.cy;

	// Save the origional scrollbar info - for CalcBars
	m_origPageDev = sizePage;
	m_origLineDev = sizeLine;

	// Fugure out scroll bar info
	CalcBars();
}

/////////////////////////////////////////////////////////////////////////////
// Getting information

void CPreView::ScrollToPosition(POINT pt)    // logical coordinates
{
	{
		HDC hdc = GetDC(NULL);
		SetMapMode(hdc, m_nMapMode);
		LPtoDP(hdc, (LPPOINT)&pt, 1);
		ReleaseDC(NULL, hdc);
	}

	// now in device coordinates - limit if out of range
	int xMax = GetScrollLimit(SB_HORZ);
	int yMax = GetScrollLimit(SB_VERT);
	if (pt.x < 0)
		pt.x = 0;
	else if (pt.x > xMax)
		pt.x = xMax;
	if (pt.y < 0)
		pt.y = 0;
	else if (pt.y > yMax)
		pt.y = yMax;

	ScrollToDevicePosition(pt);
}

CPoint CPreView::GetDeviceScrollPosition() const
{
	// スクロール バー内でのスクロール ボックスの水平、垂直方向の現在の位置を
	// デバイス単位で取得します。
	// これはドキュメントの位置に対応する座標の組で、スクロールされたビューの
	// 左上隅を示します。

	CPoint pt(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

	if (m_bCenter)
	{
		CRect rect;
		GetClientRect(m_hWnd, &rect);

		// if client area is larger than total device size,
		// the scroll positions are overridden to place origin such that
		// output is centered in the window
		// GetDeviceScrollPosition() must reflect this

		if (m_totalDev.cx < rect.Width())
			pt.x = -((rect.Width() - m_totalDev.cx) / 2);
		if (m_totalDev.cy < rect.Height())
			pt.y = -((rect.Height() - m_totalDev.cy) / 2);
	}

	return pt;
}

void CPreView::ScrollToDevicePosition(POINT ptDev)
{
	// Note: ScrollToDevicePosition can and is used to scroll out-of-range
	//  areas as far as CPreView is concerned -- specifically in
	//  the print-preview code.  Since OnScrollBy makes sure the range is
	//  valid, ScrollToDevicePosition does not vector through OnScrollBy.

	SetScrollPos(SB_HORZ, ptDev.x);
	SetScrollPos(SB_VERT, ptDev.y);
	RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
}

/////////////////////////////////////////////////////////////////////////////
//

int CPreView::OnCreate(LPCREATESTRUCT /*lpcs*/)
{
	OnInitialUpdate();
	return 0;
}

void CPreView::OnSize(UINT /*nType*/, int cx, int cy)
{
	if (m_hbmBack)
		DeleteObject(m_hbmBack);

	mif image(cx, cy, 24);
	RGBA* p = image.m_pixels;
	for (int y = 0; y < cy; y++)
		for (int x = 0; x < cx; x++)
		{
			BYTE c = (x/8 + y/8) % 2 == 0 ? 0xcc:0xff;
			p->red=p->green=p->blue=c;
			p++;
		}
	m_hbmBack = image.createbitmap(NULL);
	image.destroy();

	// UpdateBars() handles locking out recursion
	UpdateBars();
}

BOOL CPreView::OnEraseBkgnd(HDC hdc)
{
	CRect rc;
	GetClientRect(m_hWnd, &rc);

	HDC hdcw = CreateCompatibleDC(hdc);
	HBITMAP hbm = SelectBitmap(hdcw, m_hbmBack);
	BitBlt(hdc, 0, 0, rc.Width(), rc.Height(), hdcw, 0, 0, SRCCOPY);
	SelectBitmap(hdcw, hbm);
	DeleteDC(hdcw);
	return TRUE;
}

BOOL CPreView::OnSetDIB(HDIB hdib)
{
	m_bHScroll = m_bVScroll = FALSE;

	if (m_image)
		m_image->destroy();
	m_image = NULL;

	if (m_image_s)
		m_image_s->destroy();
	m_image_s = NULL;

	if (m_hbm)
		DeleteBitmap(m_hbm);
	m_hbm = NULL;

	if (m_hpal)
		DeletePalette(m_hpal);
	m_hpal = NULL;

	if (!hdib)
	{
		SetZoomSizes(CSize(0, 0));
		Invalidate(TRUE);
		return FALSE;
	}

	m_image = new mif(hdib);
	m_image_s = new mif(*m_image);

	CRect rc;
	GetClientRect(m_hWnd, &rc);
	m_image_s->zoom_inframe(rc.Width(), rc.Height());

	mif* image = m_bWhole ? m_image_s : m_image;
	m_bHScroll = (rc.Width() < image->m_width);
	m_bVScroll = (rc.Height() < image->m_height);

	m_hbm = image->createbitmap(&m_hpal);
	SetZoomSizes(CSize(image->m_width, image->m_height));

	CPoint ptCenter(image->m_width / 2, image->m_height / 2);
	CenterOnLogicalPoint(ptCenter);

	Invalidate(TRUE);
	return TRUE;
}

void CPreView::OnSetWholeView(BOOL bWhole)
{
	bWhole = !bWhole ? FALSE : TRUE;
	if (m_bWhole != bWhole)
	{
		m_bWhole = bWhole;
		if (!m_image)
			return;

		CRect rc;
		GetClientRect(m_hWnd, &rc);

		mif* image = m_bWhole ? m_image_s : m_image;
		m_bHScroll = (rc.Width() < image->m_width);
		m_bVScroll = (rc.Height() < image->m_height);

		if (m_hbm)
			DeleteBitmap(m_hbm);
		m_hbm = NULL;

		if (m_hpal)
			DeletePalette(m_hpal);
		m_hpal = NULL;

		m_hbm = image->createbitmap(&m_hpal);
		SetZoomSizes(CSize(image->m_width, image->m_height));

		CPoint ptCenter(image->m_width / 2, image->m_height / 2);
		CenterOnLogicalPoint(ptCenter);

		Invalidate(TRUE);
	}
}

void CPreView::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_hWnd, &ps);
	OnPrepareDC(hdc);
	OnDraw(hdc);
	EndPaint(m_hWnd, &ps);
}

void CPreView::OnDraw(HDC hdc)
{
	if (m_hbm)
	{
		HDC hdcw = CreateCompatibleDC(hdc);

		HPALETTE hpal = NULL, hpalw = NULL;
		if (m_hpal)
		{
			hpal = SelectPalette(hdc, m_hpal, TRUE);
			hpalw = SelectPalette(hdcw, m_hpal, TRUE);
			RealizePalette(hdc);
		}
		SetStretchBltMode(hdc, COLORONCOLOR);

		HBITMAP ho;
		if (m_hbmEffect)
		{
			CRect rc;
			GetClientRect(m_hWnd, &rc);
			DPtoLP(hdc, (LPPOINT)&rc, 2);
			ho = SelectBitmap(hdcw, m_hbmEffect);

			BITMAP bm;
			GetObject(m_hbmEffect, sizeof(BITMAP), &bm);
			BitBlt(hdc, max(0, rc.left), max(0, rc.top),
					min(bm.bmWidth, rc.Width()), min(bm.bmHeight, rc.Height()),
					hdcw, 0, 0, SRCCOPY);
		}
		else
		{
			ho = SelectBitmap(hdcw, m_hbm);
			BitBlt(hdc, 0, 0, m_totalLog.cx, m_totalLog.cy, hdcw, 0, 0, SRCCOPY);
		}

		SelectBitmap(hdcw, ho);

		if (m_hpal)
		{
			SelectPalette(hdc, hpal, FALSE);
			SelectPalette(hdcw, hpalw, FALSE);
		}
		DeleteDC(hdcw);
	}
}

void CPreView::OnPaletteChanged(HWND hwndFocus)
{
	if (m_hpal && (hwndFocus != m_hWnd))
	{
		HDC hdc = GetDC(m_hWnd);
		HPALETTE hpal = SelectPalette(hdc, m_hpal, FALSE);
		RealizePalette(hdc);
		UpdateColors(hdc);
		SelectPalette(hdc, hpal, FALSE);
		ReleaseDC(m_hWnd, hdc);
	}
}

BOOL CPreView::OnQueryNewPalette()
{
	if (m_hpal)
	{
		HDC hdc = GetDC(m_hWnd);
		HPALETTE hpal = SelectPalette(hdc, m_hpal, FALSE);
		RealizePalette(hdc);
		Invalidate(TRUE);
		SelectPalette(hdc, hpal, FALSE);
		ReleaseDC(m_hWnd, hdc);
		return TRUE;
	}
	return FALSE;
}

void CPreView::CenterOnPoint(CPoint ptCenter) // center in device coords
{
	CRect rect;
	GetClientRect(m_hWnd, &rect);           // find size of client window

	int xDesired = ptCenter.x - rect.Width() / 2;
	int yDesired = ptCenter.y - rect.Height() / 2;

	if (!m_bHScroll || xDesired < 0)
		xDesired = 0;
	else
	{
		int xMax = GetScrollLimit(SB_HORZ);
		if (xDesired > xMax)
			xDesired = xMax;
	}

	if (!m_bVScroll || yDesired < 0)
		yDesired = 0;
	else
	{
		int yMax = GetScrollLimit(SB_VERT);
		if (yDesired > yMax)
			yDesired = yMax;
	}
	SetScrollPos(SB_HORZ, xDesired);
	SetScrollPos(SB_VERT, yDesired);
}

/////////////////////////////////////////////////////////////////////////////
// Tie to scrollbars and CWnd behaviour

// スクロール・バーの状態を変更せずに、スクロール・バーの状態を返す
void CPreView::GetScrollBarState(CSize sizeClient, CSize& needSb,
	CSize& sizeRange, CPoint& ptMove, BOOL bInsideClient)
{
	// スクロールバーサイズの取得
	CSize sizeSb;
	GetScrollBarSizes(sizeSb);

	// enough room to add scrollbars
	sizeRange = m_totalDev - sizeClient;
		// > 0 => need to scroll
	ptMove = GetDeviceScrollPosition();
		// point to move to (start at current scroll pos)

	BOOL bNeedH = sizeRange.cx > 0;
	if (!bNeedH)							// 水平方向のスクロールバーが必要ではない
		ptMove.x = 0;                       // jump back to origin
	else if (bInsideClient)
		sizeRange.cy += sizeSb.cy;          // need room for a scroll bar

	BOOL bNeedV = sizeRange.cy > 0;
	if (!bNeedV)							// 垂直方向のスクロールバーが必要ではない
		ptMove.y = 0;                       // jump back to origin
	else if (bInsideClient)
		sizeRange.cx += sizeSb.cx;          // need room for a scroll bar

	if (bNeedV && !bNeedH && sizeRange.cx > 0)
	{
		// need a horizontal scrollbar after all
		bNeedH = TRUE;
		sizeRange.cy += sizeSb.cy;
	}

	// 現在のスクロール位置が限界(スクロールする制限)を過ぎている場合
	if (sizeRange.cx > 0 && ptMove.x >= sizeRange.cx)
		ptMove.x = sizeRange.cx;
	if (sizeRange.cy > 0 && ptMove.y >= sizeRange.cy)
		ptMove.y = sizeRange.cy;

	// now update the bars as appropriate
	needSb.cx = bNeedH;
	needSb.cy = bNeedV;

	// needSb, sizeRange, and ptMove area now all updated
}

void CPreView::GetScrollBarSizes(CSize& sizeSb)
{
	sizeSb.cx = sizeSb.cy = 0;
	DWORD dwStyle = GetStyle();

	// vert scrollbars will impact client area of this window
	sizeSb.cx = GetSystemMetrics(SM_CXVSCROLL);
	if (dwStyle & WS_BORDER)
		sizeSb.cx -= GetSystemMetrics(SM_CXBORDER);

	// horz scrollbars will impact client area of this window
	sizeSb.cy = GetSystemMetrics(SM_CYHSCROLL);
	if (dwStyle & WS_BORDER)
		sizeSb.cy -= GetSystemMetrics(SM_CYBORDER);
}

BOOL CPreView::GetTrueClientSize(CSize& size, CSize& sizeSb)
	// return TRUE if enough room to add scrollbars if needed
{
	CRect rect;
	GetClientRect(m_hWnd, &rect);
	size.cx = rect.right;
	size.cy = rect.bottom;

	// first get the size of the scrollbars for this window
	GetScrollBarSizes(sizeSb);

	// first calculate the size of a potential scrollbar
	// (scroll bar controls do not get turned on/off)
	if (sizeSb.cx != 0 && m_bVScroll)
	{
		// vert scrollbars will impact client area of this window
		size.cx += sizeSb.cx;   // currently on - adjust now
	}
	if (sizeSb.cy != 0 && m_bHScroll)
	{
		// horz scrollbars will impact client area of this window
		size.cy += sizeSb.cy;   // currently on - adjust now
	}

	// return TRUE if enough room
	return (size.cx > sizeSb.cx && size.cy > sizeSb.cy);
}

void CPreView::UpdateBars()
{
	// UpdateBars may cause window to be resized - ignore those resizings
	if (m_bInsideUpdate)
		return;         // Do not allow recursive calls

	// Lock out recursion
	m_bInsideUpdate = TRUE;

	// update the horizontal to reflect reality
	// NOTE: turning on/off the scrollbars will cause 'OnSize' callbacks

	CRect rectClient;

	CSize sizeClient;
	CSize sizeSb;

	// get client rect
	if (!GetTrueClientSize(sizeClient, sizeSb))
	{
		// no room for scroll bars (common for zero sized elements)
		CRect rect;
		GetClientRect(m_hWnd, &rect);
		m_bInsideUpdate = FALSE;
		return;
	}

	// enough room to add scrollbars
	CSize sizeRange;
	CPoint ptMove;
	CSize needSb;

	// get the current scroll bar state given the true client area
	GetScrollBarState(sizeClient, needSb, sizeRange, ptMove, TRUE);
	if (needSb.cx)
		sizeClient.cy -= sizeSb.cy;
	if (needSb.cy)
		sizeClient.cx -= sizeSb.cx;

	// first scroll the window as needed
	ScrollToDevicePosition(ptMove); // will set the scroll bar positions too

	// this structure needed to update the scrollbar page range
	SCROLLINFO info;
	info.cbSize = sizeof(info);
	info.fMask = SIF_PAGE|SIF_RANGE;
	info.nMin = 0;

	// now update the bars as appropriate
	if (needSb.cx)
	{
		info.nPage = sizeClient.cx;
		info.nMax = m_totalDev.cx-1;
		SetScrollInfo(SB_HORZ, &info);
	}
	if (needSb.cy)
	{
		info.nPage = sizeClient.cy;
		info.nMax = m_totalDev.cy-1;
		SetScrollInfo(SB_VERT, &info);
	}

	// remove recursion lockout
	m_bInsideUpdate = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CPreView scrolling

BOOL CPreView::OnSetCursor(HWND hwnd, UINT nHitTest, UINT /*message*/)
{
	if (nHitTest != HTCLIENT || hwnd != m_hWnd)
		return (BOOL)Default();

	::SetCursor(m_hcHandOpen);
	return TRUE;
}

void CPreView::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	Default();
	if (m_bCaptured)
	{
		m_bCaptured = FALSE;
		ReleaseCapture();
		::SetCursor(m_hcHandOpen);

		if (GetStyle() & IVS_EFFECTCALLBACK)
		{
			NotifyUpdateEffect();
			RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
		}
	}
}

void CPreView::OnLButtonDown(UINT, CPoint point)
{
	Default();

	// 最終ポイントの更新
	m_ptPrev = point;
	if (m_bVScroll || m_bHScroll)
	{
		m_bCaptured = TRUE;
		SetCapture(m_hWnd);
		::SetCursor(m_hcHandClose);

		if (m_hpalEffect)
			DeletePalette(m_hpalEffect);
		m_hpalEffect= NULL;
		if (m_hbmEffect)
			DeleteBitmap(m_hbmEffect);
		m_hbmEffect = NULL;

		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
	}
}

void CPreView::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	// Pass the message along
	Default();

	// Get the Device Context
	HDC hdc = GetDC(m_hWnd);
	OnPrepareDC(hdc);

	CPoint ptLP = point;
	DPtoLP(hdc, &ptLP, 1);			// ptLP は 画像上のピクセル位置

	if (m_bCaptured)
	{
		if (point != m_ptPrev)
		{
			CPoint pos(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));

			if (m_bHScroll)
				pos.x += m_ptPrev.x - point.x;
			if (m_bVScroll)
				pos.y += m_ptPrev.y - point.y;
			ScrollToPosition(pos);
		}
	}
	ReleaseDC(m_hWnd, hdc);

	// 最終ポイントの更新
	m_ptPrev = point;
}

BOOL CPreView::ViewDPtoLP(LPPOINT lpPoints, int nCount)
{
	// Convert to logical units
	// Called from View when no DC is available
	HDC hdc = GetWindowDC(m_hWnd);
	OnPrepareDC(hdc);
	BOOL bOK = DPtoLP(hdc, lpPoints, nCount);
	ReleaseDC(m_hWnd, hdc);
	return bOK;
}

BOOL CPreView::ViewLPtoDP(LPPOINT lpPoints, int nCount)
{
	// Convert to logical units
	// Called from View when no DC is available
	HDC hdc = GetWindowDC(m_hWnd);
	OnPrepareDC(hdc);
	BOOL bOK = LPtoDP(hdc, lpPoints, nCount);
	ReleaseDC(m_hWnd, hdc);
	return bOK;
}

void CPreView::ClientToDevice(CPoint &point)
{
	// Need to account for scrollbar position
	CPoint scrollPt = GetDeviceScrollPosition();
	point.x += scrollPt.x;
	point.y += scrollPt.y;
}

void CPreView::CenterOnLogicalPoint(CPoint pt)
{
	// Convert the point to device coordinates
	ViewLPtoDP(&pt);

	// Account for scroll bar position
	ClientToDevice(pt);

	// Use CScrollView's function for device coordinates
	CenterOnPoint(pt);
}

CPoint CPreView::GetLogicalCenterPoint (void)  // Point in logical units
{
	CPoint pt;
	CRect rect;

	// Get the center of screen
	GetClientRect(m_hWnd, &rect);
	pt.x = (rect.Width()  / 2);
	pt.y = (rect.Height() / 2);

	// Convert the point to logical coordinates
	ViewDPtoLP(&pt);
	return pt;
}

void CPreView::NotifyUpdateEffect()
{
	if (!m_image)
		return;

	// 新しい画像を問い合わせる
	UINT id = GetWindowID(m_hWnd);
	MV_GETEFFECT mve;
	memset(&mve, 0, sizeof(mve));
	mve.hdr.hwndFrom = m_hWnd;
	mve.hdr.idFrom = id;
	mve.hdr.code = MVN_GETEFFECT;

	CRect rc;
	GetClientRect(m_hWnd, &rc);

	HDC hdc = GetDC(m_hWnd);
	OnPrepareDC(hdc);
	DPtoLP(hdc, (LPPOINT)&rc, 2);
	ReleaseDC(m_hWnd, hdc);

	mif* image = m_bWhole ? m_image_s : m_image;

	int width = min(image->m_width, rc.Width());
	int height = min(image->m_height, rc.Height());

	mif work(width, height, image->m_bits);
	work.blt(0, 0, width, height, image, max(0, rc.left), max(0, rc.top));
	if (image->m_bits < 24)
		memcpy(work.m_map, image->m_map, sizeof(RGBA) * image->m_colors);
	mve.hdibSrc = work.createdib();
	work.destroy();

	SendMessage(m_hParent, WM_NOTIFY, id, (LPARAM)&mve);
	if (mve.hdibDst)
	{
		work.attachdib(mve.hdibDst);
		GlobalFree(mve.hdibDst);

		if (m_hbmEffect)
			DeleteBitmap(m_hbmEffect);
		if (m_hpalEffect)
			DeletePalette(m_hpalEffect);

		m_hbmEffect = work.createbitmap(&m_hpalEffect);
		work.destroy();
	}
	GlobalFree(mve.hdibSrc);
}
