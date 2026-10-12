// view.cpp : implementation file
//

#include "stdafx.h"
#include "lsimg.h"
#include "resource.h"


#if 0			// no more use this control

extern HINSTANCE ghInst;



#define SCL_ORG(a, b)	((DWORD)(a) * (DWORD)HIWORD((b)) / (DWORD)LOWORD((b)))
#define SCL_MAP(a, b)	((DWORD)(a) * (DWORD)LOWORD((b)) / (DWORD)HIWORD((b)))

#define ZOOM_25			(MAKELONG(1, 4))
#define ZOOM_50			(MAKELONG(1, 2))
#define ZOOM_75			(MAKELONG(2, 3))
#define ZOOM_100		(MAKELONG(1, 1))
#define ZOOM_150		(MAKELONG(3, 2))
#define ZOOM_200		(MAKELONG(2, 1))
#define ZOOM_300		(MAKELONG(3, 1))

typedef struct tagVWINFO {
	HDIB m_hdib;
	HBITMAP m_hbm;
	HPALETTE m_hpal;
	BOOL m_bSizing;
	CSize m_sdib;
	CSize m_sdsp;
	DWORD m_dws;
	COLORREF m_bkcolor;
} VWINFO;

#define GETINFO(a)		((VWINFO*)GetWindowLongPtr((a), 0))
#define SETINFO(a,b)	(SetWindowLongPtr((a), 0, (LONG_PTR)(b)))


static int vw_OnCreate(HWND hwnd, LPCREATESTRUCT pcs);
static void vw_WepData(HWND hwnd);
static void vw_OnPaletteChanged(HWND hwnd, HWND hwndFocus);
static BOOL vw_OnQueryNewPalette(HWND hwnd);
static void vw_DoSize(HWND hwnd);
static void vw_DoScroll(HWND hwnd, int msg, UINT nPos, UINT nSBCode);
static void vw_OnPaint(HWND hwnd);
static void vw_OnSetDIB(HWND hwnd, HDIB hdib, BOOL bRedraw);
static void vw_OnSetBkColor(HWND hwnd, COLORREF bkcolor, BOOL bRedraw);
static void vw_OnDestroy(HWND hwnd);
static void vw_OnSetZoom(HWND hwnd, int nZoom);
static void vw_SetupScrollBars(HWND hwnd, WORD cxBitmap, WORD cyBitmap);
static void vw_ReallyGetClientRect(HWND hwnd, CRect& rc);

LRESULT CALLBACK ViewWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
			return (LRESULT)vw_OnCreate(hwnd, (LPCREATESTRUCT)lParam);

		case WM_SETFOCUS:
			SetFocus(GetParent(hwnd));
			return 0;

		case WM_SIZE:
			vw_DoSize(hwnd);
			return 0;

		case WM_PAINT:
			vw_OnPaint(hwnd);
			return 0;

		case WM_PALETTECHANGED:
			vw_OnPaletteChanged(hwnd, (HWND)wParam);
			return 0;

		case WM_QUERYNEWPALETTE:
			return vw_OnQueryNewPalette(hwnd);

		case WM_ERASEBKGND:
			return TRUE;

		case WM_HSCROLL:
			if (GetWindowStyle(hwnd) & WS_HSCROLL)
				vw_DoScroll(hwnd, WM_HSCROLL, HIWORD(wParam), LOWORD(wParam));
			return 0;

		case WM_VSCROLL:
			if (GetWindowStyle(hwnd) & WS_VSCROLL)
				vw_DoScroll(hwnd, WM_VSCROLL, HIWORD(wParam), LOWORD(wParam));
			return 0;

		case DVM_SETDIB:
			vw_OnSetDIB(hwnd, (HDIB)lParam, (BOOL)wParam);
			return 0;

		case DVM_SETZOOM:
			vw_OnSetZoom(hwnd, (int)lParam);
			return 0;

		case DVM_SETBKCOLOR:
			vw_OnSetBkColor(hwnd, (COLORREF)lParam, (BOOL)wParam);
			return 0;

		case WM_DESTROY:
			vw_OnDestroy(hwnd);
			break;

		default:
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static int vw_OnCreate(HWND hwnd, LPCREATESTRUCT pcs)
{
	VWINFO* pvi = new VWINFO;
	memset(pvi, 0, sizeof(VWINFO));
	pvi->m_dws = MAKELONG(1, 1);
	pvi->m_bkcolor = RGB(160, 160, 160);
	SETINFO(hwnd, pvi);
	return 0;
}

static void vw_WepData(HWND hwnd)
{
	VWINFO* pvi = GETINFO(hwnd);
	if (pvi)
	{
		if (pvi->m_hdib)
			GlobalFree(pvi->m_hdib);
		if (pvi->m_hbm)
			DeleteObject(pvi->m_hbm);
		if (pvi->m_hpal)
			DeleteObject(pvi->m_hpal);

		pvi->m_hdib = NULL;
		pvi->m_hbm = NULL;
		pvi->m_hpal = NULL;
		pvi->m_bSizing = FALSE;
	}
}

static void vw_OnPaletteChanged(HWND hwnd, HWND hwndFocus)
{
	VWINFO* pvi = GETINFO(hwnd);
	if (pvi && pvi->m_hpal)
	{
		// If this application did not change the palette, select
		// and realize this application's palette
		if (hwndFocus != hwnd)
		{
			HDC hdc = GetDC(hwnd);
			HPALETTE hOldPal = SelectPalette(hdc, pvi->m_hpal, FALSE);
			RealizePalette(hdc);
			UpdateColors(hdc);
			SelectPalette(hdc, hOldPal, FALSE);
			ReleaseDC(hwnd, hdc);
		}
	}
}

static BOOL vw_OnQueryNewPalette(HWND hwnd)
{
	VWINFO* pvi = GETINFO(hwnd);
	if (pvi && pvi->m_hpal)
	{
		HDC hdc = GetDC(hwnd);
		HPALETTE hOldPal = SelectPalette(hdc, pvi->m_hpal, FALSE);
		RealizePalette(hdc);

		RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);

		SelectPalette(hdc, hOldPal, FALSE);
		ReleaseDC(hwnd, hdc);
		return TRUE;
	}
	return FALSE;
}

static void vw_DoSize(HWND hwnd)
{
	VWINFO* pvi = GETINFO(hwnd);
	if (pvi)
	{
		if (pvi->m_bSizing)
			return;

		pvi->m_bSizing = TRUE;
		if (pvi->m_hbm)
		{
			// Find out the dimensions of the window, and the current thumb
			// positions
			CRect rc;
			GetClientRect(hwnd, &rc);
			int cxScroll = GetScrollPos(hwnd, SB_HORZ);
			int cyScroll = GetScrollPos(hwnd, SB_VERT);

			// If current thumb positions would cause blank space
			// at right or bottom of client area, repaint
			if (cxScroll + rc.right > pvi->m_sdsp.cx ||
				cyScroll + rc.bottom > pvi->m_sdsp.cy)
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

			// Make sure scroll bars are updated
			vw_SetupScrollBars(hwnd, (WORD)pvi->m_sdsp.cx, (WORD)pvi->m_sdsp.cy);
		}
		pvi->m_bSizing = FALSE;
	}
}

static void vw_DoScroll(HWND hwnd, int msg, UINT nPos, UINT nSBCode)
{
	CRect rc, rcc;
	GetClientRect(hwnd, &rcc);
	rc = rcc;

	int nHorzOrVert;
	int cxClient;
	if (msg == WM_HSCROLL)
	{
		nHorzOrVert = SB_HORZ;
		cxClient = rc.Width();
	}
	else
    {
		nHorzOrVert = SB_VERT;
		cxClient = rc.Height();
	}

	int xBar = GetScrollPos(hwnd, nHorzOrVert);
	int nMin;
	int	nMax;
	GetScrollRange(hwnd, nHorzOrVert, &nMin, &nMax);

	int dx;
	switch (nSBCode)
    {
		case SB_TOP:
			dx = -cxClient;
			break;

		case SB_BOTTOM:
			dx = cxClient;
			break;

		case SB_LINEDOWN:				// 8 line right.
			dx = 8;
			break;

		case SB_LINEUP:					// 8 line left.
            dx = -8;
			break;

		case SB_PAGEDOWN:				// One page right.
			dx = cxClient / 4;
			break;

		case SB_PAGEUP:					// One page left.
			dx = -(cxClient / 4);
			break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:			// Absolute position.
			dx = nPos - xBar;
			break;

		default:						// No change.
    		dx = 0;
			break;
	}

	if (dx)
	{
		xBar += dx;
		if (xBar < nMin)
		{
			dx -= xBar - nMin;
			xBar = nMin;
		}

		if (xBar > nMax)
		{
			dx -= xBar - nMax;
			xBar = nMax;
		}

		if (dx)
		{
			dx *= -1;
			if (nHorzOrVert == SB_HORZ)
			{
				ScrollWindow(hwnd, dx, 0, NULL, &rcc);
				if (dx > 0)
					rc.left = rc.right - dx;
				else
					rc.right = rc.left + dx;
			}
			else
			{
				ScrollWindow(hwnd, 0, dx, NULL, &rcc);
				if (dx > 0)
					rc.top = rc.bottom - dx;
				else
					rc.bottom = rc.top + dx;
			}
			SetScrollPos(hwnd, nHorzOrVert, xBar, FALSE);
		}
	}
}

static void vw_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	CRect rc;
	GetClientRect(hwnd, &rc);

	VWINFO* pvi = GETINFO(hwnd);
	if (pvi)
	{
		SetBkColor(hdc, pvi->m_bkcolor);
		if (!pvi->m_hbm)
		{
			SetScrollRange(hwnd, SB_VERT, 0, 0, TRUE);
			SetScrollRange(hwnd, SB_HORZ, 0, 0, TRUE);
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
			return;
		}

		vw_SetupScrollBars(hwnd, (WORD)pvi->m_sdsp.cx, (WORD)pvi->m_sdsp.cy);

		CRect rcDDB;
		CRect rcDC;
		rcDC = rc;

		int xScroll = GetScrollPos(hwnd, SB_HORZ);
		int yScroll = GetScrollPos(hwnd, SB_VERT);

		// Set up the necessary rectangles -- i.e. the rectangle
		//  we're rendering into, and the rectangle in the bitmap
		rcDDB.left = xScroll;
		rcDDB.top = yScroll;
		rcDDB.right = xScroll + rcDC.Width();
		rcDDB.bottom = yScroll + rcDC.Height();

		if (rcDDB.right > pvi->m_sdsp.cx)
		{
			int dx;
			dx = pvi->m_sdsp.cx - rcDDB.right;
			rcDDB.right += dx;
			rcDC.right += dx;
		}

		if (rcDDB.bottom > pvi->m_sdsp.cy)
		{
			int dy;
			dy = pvi->m_sdsp.cy - rcDDB.bottom;
			rcDDB.bottom += dy;
			rcDC.bottom += dy;
		}

		rcDDB.left = SCL_ORG(rcDDB.left, pvi->m_dws);
		rcDDB.right = SCL_ORG(rcDDB.right, pvi->m_dws);
		rcDDB.top = SCL_ORG(rcDDB.top, pvi->m_dws);
		rcDDB.bottom = SCL_ORG(rcDDB.bottom, pvi->m_dws);

		imgPaintBitmap(hdc, &rcDC, pvi->m_hbm, &rcDDB, pvi->m_hpal);

		if (rcDC.right < rc.right)
		{
			rcDDB = rc;
			rcDDB.left = rcDC.right;
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDDB, NULL, 0, NULL);
		}
		if (rcDC.bottom < rc.bottom)
		{
			rcDDB = rc;
			rcDDB.top = rcDC.bottom;
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDDB, NULL, 0, NULL);
		}
	}
}

static void vw_OnSetDIB(HWND hwnd, HDIB hdib, BOOL bRedraw)
{
	VWINFO* pvi = GETINFO(hwnd);
	if (!pvi)
		return;

	CSize sizeOrg = pvi->m_sdib;
	int cxScroll = GetScrollPos(hwnd, SB_HORZ);
	int cyScroll = GetScrollPos(hwnd, SB_VERT);

	vw_WepData(hwnd);

	if (!hdib)
		return;

	pvi->m_hdib = hdib;
	BITMAPINFOHEADER* pbh = dib_lockheader(hdib);
	pvi->m_sdib.cx = pbh->biWidth;
	pvi->m_sdib.cy = pbh->biHeight;
	dib_unlock(hdib);

	// スケールサイズを取得
	pvi->m_sdsp.cx = SCL_MAP(pvi->m_sdib.cx, pvi->m_dws);
	pvi->m_sdsp.cy = SCL_MAP(pvi->m_sdib.cy, pvi->m_dws);

	imgCreateBitmapFromDIB(hdib, &pvi->m_hbm, &pvi->m_hpal);
	if (pvi->m_hbm) {
		// パレット初期化
		vw_OnPaletteChanged(hwnd, NULL);

		UINT flag = RDW_INVALIDATE | RDW_UPDATENOW;
		if (sizeOrg != pvi->m_sdib) {
			SetScrollPos(hwnd, SB_HORZ, 0, FALSE);
			SetScrollPos(hwnd, SB_VERT, 0, FALSE);
			vw_DoSize(hwnd);
			flag |= RDW_ERASE;
		}
		else if (bRedraw) {
			SetScrollPos(hwnd, SB_HORZ, cxScroll, FALSE);
			SetScrollPos(hwnd, SB_VERT, cyScroll, FALSE);
		}
		RedrawWindow(hwnd, NULL, NULL, flag);
	}
}

static void vw_OnSetBkColor(HWND hwnd, COLORREF bkcolor, BOOL bRedraw)
{
	VWINFO* pvi = GETINFO(hwnd);
	if (!pvi)
		return;

	if (pvi->m_bkcolor != bkcolor) {
		pvi->m_bkcolor = bkcolor;
		InvalidateRect(hwnd, NULL, FALSE);
	}
}

static void vw_OnDestroy(HWND hwnd)
{
	vw_WepData(hwnd);

	VWINFO* pvi = GETINFO(hwnd);
	if (pvi)
		delete pvi;
}

static void vw_OnSetZoom(HWND hwnd, int nZoom)
{
	VWINFO* pvi = GETINFO(hwnd);
	if (!pvi)
		return;

	if ((DWORD)nZoom == pvi->m_dws)
		return;

	pvi->m_dws = nZoom;
	pvi->m_sdsp.cx = SCL_MAP(pvi->m_sdib.cx, pvi->m_dws);
	pvi->m_sdsp.cy = SCL_MAP(pvi->m_sdib.cy, pvi->m_dws);

	InvalidateRect(hwnd, NULL, TRUE);
}

static void vw_SetupScrollBars(HWND hwnd, WORD cxBitmap, WORD cyBitmap)
{
	CRect rc;
	vw_ReallyGetClientRect(hwnd, rc);
	int cxWindow = rc.Width();
	int cyWindow = rc.Height();

	// Now determine if we need the scroll bars
	BOOL bNeedScrollX = FALSE;
	BOOL bNeedScrollY = FALSE;
	if (cxWindow + 2 < cxBitmap) {
		bNeedScrollX = TRUE;
		cyWindow -= GetSystemMetrics(SM_CYHSCROLL);
	}

	if (cyWindow + 2 < cyBitmap) {
		bNeedScrollY = TRUE;
		cxWindow -= GetSystemMetrics(SM_CXVSCROLL);
		if (!bNeedScrollX) {
			if (cxWindow + 2 < cxBitmap) {
				bNeedScrollX = TRUE;
				cyWindow -= GetSystemMetrics(SM_CYHSCROLL);
			}
		}
	}

	// Setup the scroll bar ranges.  We want to be able to
	// scroll the window so that all the bitmap can appear
	// within the client area.  Take into account that
	// if the opposite scroll bar is activated, it eats
	// up some client area.

	int cxRange = 0, cyRange = 0;
	if (bNeedScrollX)	cxRange = cxBitmap - cxWindow - 1;
	if (bNeedScrollY)	cyRange = cyBitmap - cyWindow - 1;

	// Set the ranges we've calculated (0 to 0 means invisible scrollbar)
	SetScrollRange(hwnd, SB_HORZ, 0, cxRange, TRUE);
	SetScrollRange(hwnd, SB_VERT, 0, cyRange, TRUE);
}

static void vw_ReallyGetClientRect(HWND hwnd, CRect& rc)
{
	GetClientRect(hwnd, &rc);

	DWORD dwStyle = GetWindowStyle(hwnd);
	if (dwStyle & WS_HSCROLL) rc.bottom += (GetSystemMetrics(SM_CYHSCROLL) - 1);
	if (dwStyle & WS_VSCROLL) rc.right  += (GetSystemMetrics(SM_CXVSCROLL) - 1);
}


#endif
