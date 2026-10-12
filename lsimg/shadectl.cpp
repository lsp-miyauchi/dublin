///////////////////////////////////////////////////////
// shade.cpp

#include "stdafx.h"
#include <shlwapi.h>
#include "lsimg.h"
#include "resource.h"

#if 0					// no more use this control


#define RGBMAX			255			// max r/g/b value is 255
#define NCOLORSHADES	64			// this many shades in gradient

static void PaintRect(HDC hdc, int x, int y, int cx, int cy, COLORREF rgb);
static HBITMAP CreateClientBitmap(HWND hwnd);


///////////////////////////////////////////////////////
// mifshade control implementation

typedef struct tagSDINFO {
	HFONT hFont;
	COLORREF rgbFore;
	COLORREF rgbBack;
	HBITMAP hbm;
} SDINFO, *LPSDINFO;

#define GETSDINFO(a)			((LPSDINFO)GetWindowLong((a), 0))
#define SETSDINFO(a,b)			(SetWindowLong((a), 0, (long)(b)))

static LRESULT ss_OnCreate(HWND hwnd, LPCREATESTRUCT lpcs);
static void ss_OnSize(HWND hwnd, UINT flg,  int cx, int cy);
static void ss_OnSetFont(HWND hwnd, HFONT hFont, BOOL bRedraw);
static void ss_OnSetText(HWND hwnd, LPCSTR pszTitle);
static void ss_OnPaint(HWND hwnd);
static void ss_OnDestroy(HWND hwnd);
static void ss_OnSetTextColor(HWND hwnd, COLORREF rgb);
static void ss_OnSetBkColor(HWND hwnd, COLORREF rgb);

LRESULT CALLBACK ShadeWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
			return ss_OnCreate(hwnd, (LPCREATESTRUCT)lParam);

		case WM_SIZE:
			ss_OnSize(hwnd, (UINT)wParam, (int)LOWORD(lParam), (int)HIWORD(lParam));
			return 0;

		case WM_GETDLGCODE:
			return DLGC_STATIC;

		case WM_SETFONT:
			ss_OnSetFont(hwnd, (HFONT)wParam, (BOOL)LOWORD(lParam));
			return 0;

		case WM_SETTEXT:
			ss_OnSetText(hwnd, (LPCSTR)lParam);
			break;

		case WM_PAINT:
			ss_OnPaint(hwnd);
			return 0;

		case WM_ERASEBKGND:
			return (LRESULT)TRUE;

		case WM_DESTROY:
			ss_OnDestroy(hwnd);
			break;

		case MSM_SETTEXTCOLOR:
			ss_OnSetTextColor(hwnd, (COLORREF)lParam);
			return 0;

		case MSM_SETBKCOLOR:
			ss_OnSetBkColor(hwnd, (COLORREF)lParam);
			return 0;

		default:
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static LRESULT ss_OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
	SDINFO* psi = new SDINFO;
	memset(psi, 0, sizeof(SDINFO));
	psi->rgbFore = RGB(255, 255, 255);
	psi->rgbBack = GetSysColor(COLOR_ACTIVECAPTION);
	SETSDINFO(hwnd, psi);
	return 1;
}

static void ss_OnSize(HWND hwnd, UINT flg,  int cx, int cy)
{
	if (flg != SIZE_MINIMIZED)
	{
		SDINFO* psi = GETSDINFO(hwnd);
		if (psi->hbm)
		{
			DeleteObject(psi->hbm);
			psi->hbm = CreateClientBitmap(hwnd);
		}
	}
}

static void ss_OnSetFont(HWND hwnd, HFONT hFont, BOOL bRedraw)
{
	SDINFO* psi = GETSDINFO(hwnd);
	psi->hFont = (hFont != NULL ) ? hFont : GetStockFont(SYSTEM_FONT);
	if (psi->hbm)
	{
		DeleteObject(psi->hbm);
		psi->hbm = CreateClientBitmap(hwnd);
	}

	if (bRedraw)
		InvalidateRect(hwnd, NULL, TRUE);
}

static void ss_OnSetText(HWND hwnd, LPCSTR pszTitle)
{
	SDINFO* psi = GETSDINFO(hwnd);
	if (psi->hbm)
	{
		DeleteObject(psi->hbm);
		psi->hbm = CreateClientBitmap(hwnd);
	}
	InvalidateRect(hwnd, NULL, TRUE);
}

static void ss_OnPaint(HWND hwnd)
{
	SDINFO* psi = GETSDINFO(hwnd);

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	if (!psi->hbm)
		psi->hbm = CreateClientBitmap(hwnd);
	imgDrawBitmap(hdc, 0, 0, psi->hbm, NULL);
	EndPaint(hwnd, &ps);
}

static void ss_OnDestroy(HWND hwnd)
{
	SDINFO* psi = GETSDINFO(hwnd);
	if (psi->hbm)
		DeleteObject(psi->hbm);
	delete psi;
}

static void ss_OnSetTextColor(HWND hwnd, COLORREF rgb)
{
	SDINFO* psi = GETSDINFO(hwnd);
	psi->rgbFore = rgb;
	if (psi->hbm)
	{
		DeleteObject(psi->hbm);
		psi->hbm = CreateClientBitmap(hwnd);
	}
	InvalidateRect(hwnd, NULL, TRUE);
}

static void ss_OnSetBkColor(HWND hwnd, COLORREF rgb)
{
	SDINFO* psi = GETSDINFO(hwnd);
	psi->rgbBack = rgb;
	if (psi->hbm)
	{
		DeleteObject(psi->hbm);
		psi->hbm = CreateClientBitmap(hwnd);
	}
	InvalidateRect(hwnd, NULL, TRUE);
}


static void PaintRect(HDC hdc, int x, int y, int cx, int cy, COLORREF rgb)
{
	HBRUSH hbr = SelectBrush(hdc, CreateSolidBrush(rgb));
	PatBlt(hdc, x, y, cx, cy, PATCOPY);
	DeleteObject(SelectObject(hdc, hbr));
}

static HBITMAP CreateClientBitmap(HWND hwnd)
{
	SDINFO* psi = GETSDINFO(hwnd);

	DWORD dwStyle = GetWindowStyle(hwnd);

	CRect rc;
	GetClientRect(hwnd, &rc);

	HDC hdcWin = GetWindowDC(hwnd);
	HDC hdc = CreateCompatibleDC(hdcWin);
	HBITMAP hbm = CreateCompatibleBitmap(hdcWin, rc.Width(), rc.Height());
	HBITMAP hbmOld = SelectBitmap(hdc, hbm);

	// Paint shaded background. Note all painting is into memory DC,
	// not window, so there's no flicker.
	// Active caption: do shading

	int r, g, b, x, w, xDelta, wx;
	r = GetRValue(psi->rgbBack);
	g = GetGValue(psi->rgbBack);
	b = GetBValue(psi->rgbBack);
	x = rc.right;						// start the way right
	w = x - rc.left;					// width of area to shade
	xDelta = max(w / NCOLORSHADES, 1);	// width of one shade band

	if ((dwStyle & MSS_SHADEMASK) == MSS_SHADELEFT)
	{
		// paint bands right to left
		while (x > xDelta)
		{
			x -= xDelta;						// next band
			wx = w - x;
			PaintRect(hdc, rc.right - x - xDelta, 0, xDelta, rc.Height(),
					RGB(r - (r * wx) / w, g - (g * wx) / w, b - (b * wx) / w));
		}

		// whatever's left ==> black
		PaintRect(hdc, rc.right - x, 0, x, rc.Height(), RGB(0, 0, 0));
	}
	else // if ((dwStyle & MSS_SHADEMASK) == MSS_SHADERIGHT)
	{
		// paint bands right to left
		while (x > xDelta)
		{
			x -= xDelta;							// next band
			wx = w - x;
			PaintRect(hdc, x, 0, xDelta, rc.Height(), RGB(r - (r * wx) / w, g - (g * wx) / w, b - (b * wx) / w));
		}

		// whatever's left ==> black
		PaintRect(hdc, 0, 0, x, rc.Height(), RGB(0, 0, 0));
	}

	TCHAR tit[MAX_PATH];
	if (GetWindowText(hwnd, tit, MAX_PATH) > 0) {
		HFONT hf = NULL;

		if (psi->hFont)
			hf = SelectFont(hdc, psi->hFont);
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, psi->rgbFore);

		// 左右3ピクセルの空きを確保
		rc.InflateRect(-3, 0);
		UINT flg = DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;

		TCHAR* p = StrRChr(tit, NULL, '\t');
		if (p)
		{
			*p++ = '\0';
			DrawText(hdc, tit, -1, &rc, flg | DT_LEFT);
			DrawText(hdc, p, -1, &rc, flg | DT_RIGHT);
		}
		else
		{
			if ((dwStyle & MSS_TYPEMASK) == SS_RIGHT)
				flg |= DT_RIGHT;
			else if ((dwStyle & MSS_TYPEMASK) == SS_CENTER)
				flg |= DT_CENTER;
			else
				flg |= DT_LEFT;
			DrawText(hdc, tit, -1, &rc, flg);
		}
		if (hf)
			SelectObject(hdc, hf);
	}

	SelectObject(hdc, hbmOld);
	ReleaseDC(hwnd, hdcWin);
	DeleteDC(hdc);

	return hbm;
}


#endif
