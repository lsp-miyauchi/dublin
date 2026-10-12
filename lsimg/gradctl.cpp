///////////////////////////////////////////////////////
// gradctl.cpp

#include "stdafx.h"
#include "lsimg.h"
#include "common.h"
#include "resource.h"

#if 0				// no more use this control


#define MAXNUMCOLORS	8

typedef struct tagGDINFO {
	int numcolors;
	COLORREF rgbs[MAXNUMCOLORS];
	HBITMAP hbm;
	HPALETTE hpal;
} GDINFO, *LPGDINFO;

#define GETINFO(a)			((LPGDINFO)GetWindowLong((a), 0))
#define SETINFO(a,b)			(SetWindowLong((a), 0, (long)(b)))

static LRESULT gd_OnCreate(HWND hwnd, LPCREATESTRUCT lpcs);
static void gd_OnSize(HWND hwnd, UINT flg,  int cx, int cy);
static void gd_OnSetFont(HWND hwnd, HFONT hFont, BOOL bRedraw);
static void gd_OnPaint(HWND hwnd);
static void gd_OnDestroy(HWND hwnd);
static void gd_OnSetColors(HWND hwnd, int numcolors, COLORREF* rgbs);
static void gd_OnPaletteChanged(HWND hwnd, HWND hwndFocus);
static BOOL gd_OnQueryNewPalette(HWND hwnd);
static BOOL CreateGradation(HWND hwnd, HBITMAP* phbm, HPALETTE* phpal);


static BOOL grad_table(COLORREF* rgbs, int numcolors, PALETTEENTRY* ppe, int numpalettes)
{
	if (numcolors < 1 || numpalettes < 1 || !rgbs || !ppe)
		return FALSE;

	memset(ppe, 0, sizeof(PALETTEENTRY) * numpalettes);
	register PALETTEENTRY* pw = ppe;

	int i;
	if (numcolors == 1)
	{
		for (i = 0; i < numpalettes; i++)
		{
			pw->peRed = GetRValue(*rgbs);
			pw->peGreen = GetGValue(*rgbs);
			pw->peBlue = GetBValue(*rgbs);
			pw++;
		}
	}
	else
	{
		int r1, r2, g1, g2, b1, b2;
		double dr, dg, db;

		int delta_loop = numpalettes / (numcolors - 1);
		int startpos = 0, endpos;
		int left = numpalettes;
		double delta_div = (double)(delta_loop - 1);

		for (int n = 0; n < numcolors - 1; n++)
		{
			r1 = GetRValue(*rgbs);
			g1 = GetGValue(*rgbs);
			b1 = GetBValue(*rgbs);
			rgbs++;
			r2 = GetRValue(*rgbs);
			g2 = GetGValue(*rgbs);
			b2 = GetBValue(*rgbs);

			endpos = startpos + delta_loop;
			if (endpos + delta_loop > numpalettes)
				endpos = numpalettes;

			dr = (double)((r2 - r1) / delta_div);
			dg = (double)((g2 - g1) / delta_div);
			db = (double)((b2 - b1) / delta_div);

			for (i = startpos; i < endpos; i++)
			{
				pw->peRed = ADJRGB(r1 + (dr * (i - startpos)));
				pw->peGreen = ADJRGB(g1 + (dg * (i - startpos)));
				pw->peBlue = ADJRGB(b1 + (db * (i - startpos)));
				pw++;
			}
			startpos += delta_loop;
		}
	}
	return TRUE;
}

LRESULT CALLBACK GradationWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
			return gd_OnCreate(hwnd, (LPCREATESTRUCT)lParam);

		case WM_SIZE:
			gd_OnSize(hwnd, (UINT)wParam, (int)LOWORD(lParam), (int)HIWORD(lParam));
			return 0;

		case WM_GETDLGCODE:
			return DLGC_STATIC;

		case WM_PAINT:
			gd_OnPaint(hwnd);
			return 0;

		case WM_ERASEBKGND:
			return (LRESULT)TRUE;

		case WM_PALETTECHANGED:
			gd_OnPaletteChanged(hwnd, (HWND)wParam);
			return 0;

		case WM_QUERYNEWPALETTE:
			return gd_OnQueryNewPalette(hwnd);

		case WM_DESTROY:
			gd_OnDestroy(hwnd);
			break;

		case GDM_SETCOLORS:
			gd_OnSetColors(hwnd, (int)wParam, (COLORREF*)lParam);
			return 0;

		default:
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static LRESULT gd_OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
	GDINFO* pi = new GDINFO;
	memset(pi, 0, sizeof(GDINFO));
	pi->numcolors = 1;
	pi->rgbs[0] = RGB(255, 255, 255);
	SETINFO(hwnd, pi);
	return 1;
}

static void gd_OnSize(HWND hwnd, UINT flg,  int cx, int cy)
{
	if (flg != SIZE_MINIMIZED)
	{
		GDINFO* pi = GETINFO(hwnd);
		if (pi->hbm)
			DeleteObject(pi->hbm);
		if (pi->hpal)
			DeleteObject(pi->hpal);
		pi->hbm = NULL;
		pi->hpal = NULL;
		CreateGradation(hwnd, &pi->hbm, &pi->hpal);
	}
}

static void gd_OnPaint(HWND hwnd)
{
	GDINFO* pi = GETINFO(hwnd);
	if (!pi->hbm)
	{
		if (CreateGradation(hwnd, &pi->hbm, &pi->hpal) || !pi->hbm)
			return;
	}

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	imgDrawBitmap(hdc, 0, 0, pi->hbm, pi->hpal);
	EndPaint(hwnd, &ps);
}

static void gd_OnDestroy(HWND hwnd)
{
	GDINFO* pi = GETINFO(hwnd);
	if (pi->hbm)
		DeleteObject(pi->hbm);
	if (pi->hpal)
		DeleteObject(pi->hpal);
	delete pi;
}

static void gd_OnSetColors(HWND hwnd, int numcolors, COLORREF* rgbs)
{
	GDINFO* pi = GETINFO(hwnd);
	if (pi->hbm)
		DeleteObject(pi->hbm);
	if (pi->hpal)
		DeleteObject(pi->hpal);
	pi->hbm = NULL;
	pi->hpal = NULL;

	pi->numcolors = min(numcolors, MAXNUMCOLORS);
	memcpy(pi->rgbs, rgbs, sizeof(COLORREF) * numcolors);
	if (CreateGradation(hwnd, &pi->hbm, &pi->hpal) && pi->hbm)
		InvalidateRect(hwnd, NULL, TRUE);
}

static BOOL CreateGradation(HWND hwnd, HBITMAP* phbm, HPALETTE* phpal)
{
	GDINFO* pi = GETINFO(hwnd);

	CRect rc;
	GetClientRect(hwnd, &rc);

	PALETTEENTRY tbl[256];
	grad_table(pi->rgbs, pi->numcolors, tbl, 256);

	int width = rc.Width();
	int height = rc.Height();
	HDIB hdib = dib_create(width, height, 24);
	BITMAPINFO* pbi = dib_lockinfo(hdib);
	BYTE* base = dib_findbits(pbi);
	int widthbytes = WIDTHBYTES(width * 24);
	int x_div = (width > 1) ? width - 1 : 1;
	int y_div = (height > 1) ? height - 1 : 1;

	for (int y = 0; y < height; y++) {
		BYTE* p = base + (height - y - 1) * widthbytes;
		for (int x = 0; x < width; x++) {
			int r = x * 255 / x_div;
			*p++ = tbl[r].peBlue;
			*p++ = tbl[r].peGreen;
			*p++ = tbl[r].peRed;
		}
	}
	dib_unlock(hdib);

	BOOL bOK = imgCreateBitmapFromDIB(hdib, phbm, phpal);
	GlobalFree(hdib);
	return bOK;
}

static void gd_OnPaletteChanged(HWND hwnd, HWND hwndFocus)
{
	GDINFO* pi = GETINFO(hwnd);
	if (pi->hpal && hwndFocus != hwnd) {
		HDC hdc = GetDC(hwnd);
		HPALETTE hold = SelectPalette(hdc, pi->hpal, FALSE);
		RealizePalette(hdc);
		UpdateColors(hdc);
		SelectPalette(hdc, hold, FALSE);
		ReleaseDC(hwnd, hdc);
	}
}

static BOOL gd_OnQueryNewPalette(HWND hwnd)
{
	GDINFO* pi = GETINFO(hwnd);
	if (pi->hpal) {
		HDC hdc = GetDC(hwnd);
		HPALETTE hold = SelectPalette(hdc, pi->hpal, FALSE);
		RealizePalette(hdc);
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		SelectPalette(hdc, hold, FALSE);
		ReleaseDC(hwnd, hdc);
		return TRUE;
	}
	return FALSE;
}

#endif
