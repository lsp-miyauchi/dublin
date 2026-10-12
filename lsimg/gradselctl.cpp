// gradselctl : カラーピッカーのグラデーションセレクタ
//

#include "stdafx.h"
#include "lsimg.h"
#include "dib_sup.h"
#include "resource.h"


extern HINSTANCE ghInst;

///////////////////////////////////////////////////////
//

enum {
	GSM_HUE,
	GSM_SATURATION,
	GSM_LIGHTNESS,
	GSM_RED,
	GSM_GREEN,
	GSM_BLUE,
};

typedef struct tagGRADSELINFO {
	HDIB hdib;
	HBITMAP hbm;
	HPALETTE hpal;
	COLORREF ref;
	int gmode;
} GRADSELINFO;

///////////////////////////////////////////////////////
// Preview control implementation

#define GETINFO(n)			((GRADSELINFO*)GetWindowLongPtr((n), 0))
#define SETINFO(n,x)		(SetWindowLong((n), 0, (LONG_PTR)(x)))

static LRESULT gs_OnLButtonDown(HWND hwnd, UINT nFlags, CPoint pt);
static int gs_OnCreate(HWND hwnd, LPCREATESTRUCT lpcs);
static void gs_OnSize(HWND hwnd, UINT type, int cx, int cy);
static void gs_OnPaint(HWND hwnd);
static void gs_OnPaletteChanged(HWND hwnd, HWND hwndFocus);
static BOOL gs_OnQueryNewPalette(HWND hwnd);
static void gs_OnDestroy(HWND hwnd);
static LRESULT gs_OnSetRange(HWND hwnd, LPRECT prc, BOOL bRedraw);
static LRESULT gs_OnSetPos(HWND hwnd, CPoint pt);
static LRESULT gs_OnSetDIB(HWND hwnd, HDIB hDIB, BOOL bRedraw);
static LRESULT gs_OnSetBkColor(HWND hwnd, COLORREF rgb, BOOL bRedraw);
static LRESULT gs_OnUpdateEffect(HWND hwnd, BOOL bRedraw);
static BOOL gs_OnEraseBkgnd(HWND hwnd, HDC hdc);
static void gs_TrackMouse(HWND hwnd, CPoint pt);
static void gs_DoUpdateEffect(HWND hwnd);
static void gs_AdjustFrame(HWND hwnd);

LRESULT CALLBACK GradSelWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_LBUTTONDOWN:
			return gs_OnLButtonDown(hwnd, (UINT)wParam, CPoint(lParam));

		case WM_NCCREATE:
			((LPCREATESTRUCT)lParam)->dwExStyle |= WS_EX_CLIENTEDGE;
			return TRUE;

		case WM_CREATE:
			return gs_OnCreate(hwnd, (LPCREATESTRUCT)lParam);

		case WM_SIZE:
			gs_OnSize(hwnd, (UINT)wParam, (int)LOWORD(lParam), (int)HIWORD(lParam));
			break;

		case WM_PAINT:
			gs_OnPaint(hwnd);
			return 0;

		case WM_PALETTECHANGED:
			gs_OnPaletteChanged(hwnd, (HWND)wParam);
			return 0;

		case WM_QUERYNEWPALETTE:
			return gs_OnQueryNewPalette(hwnd);

		case WM_ERASEBKGND:
			return gs_OnEraseBkgnd(hwnd, (HDC)wParam);

		case WM_DESTROY:
			gs_OnDestroy(hwnd);
			break;

		default:
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static LRESULT gs_OnLButtonDown(HWND hwnd, UINT nFlags, CPoint pt)
{
	GRADSELINFO* info = GETINFO(hwnd);
	if (GetWindowStyle(hwnd) & IVS_EFFECTCALLBACK) {
		UINT id = GetWindowID(hwnd);

		GS_LBUTTONDOWN g;
		memset(&g, 0, sizeof(g));
		g.hdr.hwndFrom = hwnd;
		g.hdr.idFrom = id;
		g.hdr.code = GSN_LBUTTONDOWN;
		if (!SendMessage(GetParent(hwnd), WM_NOTIFY, id, (LPARAM)&g)) {
			gs_TrackMouse(hwnd, pt);
			return 0;
		}
	}
	return DefWindowProc(hwnd, WM_LBUTTONDOWN,
						(WPARAM)nFlags, MAKELPARAM(pt.x, pt.y));
}

static int gs_OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
	GRADSELINFO* info = new MVINFO;
	memset(info, 0, sizeof(MVINFO));
	info->rgbBkgnd = RGB(64,128,128);
	info->hcur[0] = LoadCursor(ghInst, MAKEINTRESOURCE(IDC_HANDOPEN));
	info->hcur[1] = LoadCursor(ghInst, MAKEINTRESOURCE(IDC_HANDCLOSE));
	SETINFO(hwnd, info);

	if (lpcs->style & IVS_EFFECTCALLBACK)
		SetClassLong(hwnd, GCL_HCURSOR, (long)info->hcur[0]);

	return 0;
}

static void gs_CreateGradationBitmap(HWND hwnd, int cx, int cy)
{
	GRADSELINFO* info = GETINFO(hwnd);
	if (info->hbm)
		DeleteObject(info->hbm);

	int r = GetRValue(info->ref);
	int g = GetGValue(info->ref);
	int b = GetBValue(info->ref);

	int h = r;
	int l = g;
	int s = b;
	rgb_to_hsv(&h, &l, &s);

	RGBA* line = new RGBA[cy];
	RGBA* p = line;
	int i;
	double delta;

	switch (info->gmode) {
	case GSM_HUE:
		delta = (double)cy / 254.0;
		l = g;
		s = b;
		for (i = 0; i < cy; i++) {
			r = (int)(254 - delta * (double)i);
			hls_to_rgb (&r, &g, &b);
			p->red = r;
			p->green = g;
			p->blue = b;
			p++;
		}
		break;

	case GSM_SATURATION:
		delta = (double)cy / 255.0;
		r = h;
		l = g;
		for (i = 0; i < cy; i++) {
			b = (int)(255 - delta * (double)i);
			hls_to_rgb (&r, &g, &b);
			p->red = r;
			p->green = g;
			p->blue = b;
			p++;
		}
		break;

	case GSM_LIGHTNESS:
		delta = (double)cy / 255.0;
		r = h;
		b = s;
		for (i = 0; i < cy; i++) {
			g = (int)(255 - delta * (double)i);
			hls_to_rgb (&r, &g, &b);
			p->red = r;
			p->green = g;
			p->blue = b;
			p++;
		}
		break;

	case GSM_RED:
		delta = (double)cy / 255.0;
		for (i = 0; i < cy; i++) {
			p->red = (int)(255 - delta * (double)i);
			p->green = g;
			p->blue = b;
			p++;
		}
		break;

	case GSM_GREEN:
		delta = (double)cy / 255.0;
		for (i = 0; i < cy; i++) {
			p->red = r;
			p->green = (int)(255 - delta * (double)i);
			p->blue = b;
			p++;
		}
		break;

	case GSM_BLUE:
		delta = (double)cy / 255.0;
		for (i = 0; i < cy; i++) {
			p->red = r;
			p->green = g;
			p->blue = (int)(255 - delta * (double)i);
			p++;
		}
		break;
	}

	mif image(cx, cy, 24);
	p = line;
	RGBA* q = image.m_pixels;
	for (int y = 0; y < cy; y++) {
		for (int x = 0; x < cx; x++) {
			*q++ = *p;
		}
		p++;
	}
	delete [] line;
	info->hbm = image.createbitmap(NULL);
}

static void gs_OnSize(HWND hwnd, UINT type, int cx, int cy)
{
	gs_CreateGradationBitmap(hwnd, cx, cy)
}

static void gs_OnPaint(HWND hwnd)
{
	GRADSELINFO* info = GETINFO(hwnd);

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	BITMAP bm;
	CRect rc;
	GetClientRect(hwnd, &rc);

	info->bUseEffect = FALSE;
	if (GetWindowStyle(hwnd) & IVS_EFFECTCALLBACK)
	{
		if (info->hbmEffect)
		{
			info->bUseEffect = TRUE;
			GetObject(info->hbmEffect, sizeof(BITMAP), &bm);
			imgDrawBitmap(hdc,
					(rc.left + rc.right - bm.bmWidth) / 2,
					(rc.top + rc.bottom - bm.bmHeight) / 2,
					info->hbmEffect, info->hpalEffect);
		}
		else
			imgPaintBitmap(hdc, &rc, info->hbm, &info->rc, info->hpal);
	}
	else if (info->hbm)
	{
		GetObject(info->hbm, sizeof(BITMAP), &bm);
		imgDrawBitmap(hdc,
					(rc.left + rc.right - bm.bmWidth) / 2,
					(rc.top + rc.bottom - bm.bmHeight) / 2,
					info->hbm, info->hpal);
	}
	EndPaint(hwnd, &ps);
}

static void gs_OnPaletteChanged(HWND hwnd, HWND hwndFocus)
{
	GRADSELINFO* info = GETINFO(hwnd);

	if (info->hpal) {
		if (hwnd != hwndFocus) {
			HDC hdc = GetDC(hwnd);
			HPALETTE hpalw = SelectPalette(hdc, info->hpal, FALSE);
			RealizePalette(hdc);
			UpdateColors(hdc);
			SelectPalette(hdc, hpalw, FALSE);
			ReleaseDC(hwnd, hdc);
		}
	}
}

static BOOL gs_OnQueryNewPalette(HWND hwnd)
{
	GRADSELINFO* info = GETINFO(hwnd);

	if (info->hpal) {
		HDC hdc = GetDC(hwnd);
		HPALETTE hpalw = SelectPalette(hdc, info->hpal, FALSE);
		RealizePalette(hdc);

		InvalidateRect(hwnd, NULL, TRUE);
		UpdateWindow(hwnd);

		SelectPalette(hdc, hpalw, FALSE);
		ReleaseDC(hwnd, hdc);
		return TRUE;
	}
	return FALSE;
}

static BOOL gs_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
	GRADSELINFO* info = GETINFO(hwnd);
	CRect rc;
	GetClientRect(hwnd, &rc);

	if (info->hbm) {
		HDC hdcw = CreateCompatibleDC(hdc);
		HBITMAP hbm = SelectBitmap(hdcw, info->hbm);
		BitBlt(hdc, 0, 0, rc.Width(), rc.Height(), hdcw, 0, 0, SRCCOPY);
		SelectBitmap(hdcw, hbm);
		DeleteDC(hdcw);
	}
	else {
		COLORREF cr = SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
		SetBkColor(hdc, cr);
	}
	return TRUE;
}

static void gs_OnDestroy(HWND hwnd)
{
	GRADSELINFO* info = GETINFO(hwnd);

	if (info->hbm)
		DeleteObject(info->hbm);
	if (info->hpal)
		DeleteObject(info->hpal);

	delete info;
}

static void gs_TrackMouse(HWND hwnd, CPoint pt)
{
	GRADSELINFO* info = GETINFO(hwnd);

	CRect rcRange, rcc, rcFrame, rc, rcw;
	GetClientRect(hwnd, &rcc);
	rcFrame = info->rcFrame;

	if (rcc.Width() > rcFrame.Width() && rcc.Height() > rcFrame.Height())
		return;

	// 基本位置
	CPoint ptOrg = pt;

	// 可動範囲
	rcRange.left = 0;
	rcRange.right = rcFrame.Width() * 2 - rcc.Width();
	rcRange.top = 0;
	rcRange.bottom = rcFrame.Height() * 2 - rcc.Height();
	rcRange.OffsetRect(rcc.Width() - rcFrame.Width(),
						rcc.Height() - rcFrame.Height());

	// 現在の矩形位置
	rcFrame.OffsetRect(-info->pt.x, -info->pt.y);

	SetCapture(hwnd);
	SetCursor(info->hcur[1]);
	info->bCaptured = TRUE;

	HDC hdc = GetDC(hwnd);
	for (;;)
	{
		MSG msg;
		CPoint ptw;

		WaitMessage();
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			if (msg.message < WM_MOUSEFIRST || msg.message > WM_MOUSELAST)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				continue;
			}

			ptw = CPoint(msg.lParam);

			rc = rcFrame;
			rc.OffsetRect(ptw.x - ptOrg.x, ptw.y - ptOrg.y);
			if (rc.left < rcRange.left)
				rc.OffsetRect(rcRange.left - rc.left, 0);
			else if (rc.right > rcRange.right)
				rc.OffsetRect(rcRange.right - rc.right, 0);

			if (rc.top < rcRange.top)
				rc.OffsetRect(0, rcRange.top - rc.top);
			else if (rc.bottom > rcRange.bottom)
				rc.OffsetRect(0, rcRange.bottom - rc.bottom);
			rcw = rcc;
			rcw.OffsetRect(-rc.left, -rc.top);

			if (rcw != info->rc)
			{
				info->rc = rcw;
				imgPaintBitmap(hdc, &rcc, info->hbm, &info->rc, info->hpal);

				MV_CHANGEVIEW mvc;
				memset(&mvc, 0, sizeof(mvc));
				mvc.hdr.hwndFrom = hwnd;
				mvc.hdr.idFrom = GetWindowID(hwnd);
				mvc.hdr.code = MVN_CHANGEVIEW;
				mvc.ptTopLeft.x = info->rc.left;
				mvc.ptTopLeft.y = info->rc.top;
				SendMessage(GetParent(hwnd), WM_NOTIFY,
								mvc.hdr.idFrom, (LPARAM)&mvc);
			}

			if (msg.message == WM_LBUTTONUP)
				break;
		}
		else
			continue;
	}
	ReleaseDC(hwnd, hdc);
	ReleaseCapture();

	gs_OnSetPos(hwnd, CPoint(info->rc.left, info->rc.top));
	info->bCaptured = FALSE;

	gs_DoUpdateEffect(hwnd);
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

static void gs_DoUpdateEffect(HWND hwnd)
{
	GRADSELINFO* info = GETINFO(hwnd);

	if (!info->hdib)
		return;

	// 新しい画像を問い合わせる
	UINT id = GetWindowID(hwnd);
	MV_GETEFFECT mve;
	memset(&mve, 0, sizeof(mve));
	mve.hdr.hwndFrom = hwnd;
	mve.hdr.idFrom = id;
	mve.hdr.code = MVN_GETEFFECT;

	mif image(info->hdib);
	image.crop(&info->rc);
	mve.hdibSrc = image.createdib();
	image.destroy();

	SendMessage(GetParent(hwnd), WM_NOTIFY, id, (LPARAM)&mve);
	if (mve.hdibDst)
	{
		if (info->hbmEffect)
		{
			DeleteBitmap(info->hbmEffect);
			info->hbmEffect = NULL;
		}
		if (info->hpalEffect)
		{
			DeletePalette(info->hpalEffect);
			info->hpalEffect = NULL;
		}

		imgCreateBitmapFromDIB(mve.hdibDst, &info->hbmEffect, &info->hpalEffect);
		GlobalFree(mve.hdibSrc);
		GlobalFree(mve.hdibDst);
	}
}

static void gs_AdjustFrame(HWND hwnd)
{
	GRADSELINFO* info = GETINFO(hwnd);

	CRect rcc, rcw;
	GetClientRect(hwnd, &rcc);
	CRect rc = info->rcFrame;
	if (rc.Width() < rcc.Width() || rc.Height() < rcc.Height())
	{
		int nx = 0;
		int ny = 0;
		if (rc.Width() < rcc.Width())
			nx = (rcc.Width() - rc.Width() + 1) / 2;
		if (rc.Height() < rcc.Height())
			ny = (rcc.Height() - rc.Height() + 1) / 2;

		GetWindowRect(hwnd, &rcw);
		rcw.InflateRect(-nx, -ny);

		HWND hParent = GetParent(hwnd);
		ScreenToClient(hParent, (LPPOINT)&rcw.left);
		ScreenToClient(hParent, (LPPOINT)&rcw.right);

		MoveWindow(hwnd, rcw.left, rcw.top, rcw.Width(), rcw.Height(), TRUE);
	}
}
