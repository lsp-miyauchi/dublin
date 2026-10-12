// preview.cpp : implementation file
//

#include "stdafx.h"
#include "lsimg.h"
#include "dib_sup.h"
#include "resource.h"


extern HINSTANCE ghInst;

#if 0			// no more use this control


///////////////////////////////////////////////////////
//

typedef struct tagMVINFO {
	HDIB hdib;
	int width;
	int height;
	int bits;
	int colors;
	HBITMAP hbm;
	HBITMAP hbmEffect;
	HPALETTE hpal;
	HPALETTE hpalEffect;
	HBITMAP hbmBack;
	COLORREF rgbBkgnd;
	RECT rc;
	RECT rcFrame;
	POINT pt;
	HCURSOR hcur[2];
	BOOL bCaptured;
	BOOL bUseEffect;
	int zoom;						// 25,50,75,80,100,150,200,300,400
} MVINFO;

static void WINAPI PatB(HDC hdc, int x, int y, int dx, int dy, COLORREF rgb)
{
	CRect rc;

	SetBkColor(hdc, rgb);
	rc.SetRect(x, y, x + dx, y + dy);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
}

///////////////////////////////////////////////////////
// Preview control implementation

#define GETMVINFO(n)			((MVINFO*)GetWindowLong((n), 0))
#define SETMVINFO(n,x)			(SetWindowLong((n), 0, (long)(x)))

static BOOL mv_OnSetCursor(HWND hwnd, HWND hwndCursor, UINT nHitTest, UINT message);
static LRESULT mv_OnLButtonDown(HWND hwnd, UINT nFlags, CPoint pt);
static int mv_OnCreate(HWND hwnd, LPCREATESTRUCT lpcs);
static void mv_OnSize(HWND hwnd, UINT type, int cx, int cy);
static void mv_OnPaint(HWND hwnd);
static void mv_OnPaletteChanged(HWND hwnd, HWND hwndFocus);
static BOOL mv_OnQueryNewPalette(HWND hwnd);
static void mv_OnDestroy(HWND hwnd);
static LRESULT mv_OnSetRange(HWND hwnd, LPRECT prc, BOOL bRedraw);
static LRESULT mv_OnSetPos(HWND hwnd, CPoint pt);
static LRESULT mv_OnSetDIB(HWND hwnd, HDIB hDIB, BOOL bRedraw);
static LRESULT mv_OnSetBkColor(HWND hwnd, COLORREF rgb, BOOL bRedraw);
static LRESULT mv_OnUpdateEffect(HWND hwnd, BOOL bRedraw);
static BOOL mv_OnEraseBkgnd(HWND hwnd, HDC hdc);
static void mv_TrackMouse(HWND hwnd, CPoint pt);
static void mv_DoUpdateEffect(HWND hwnd);
static void mv_AdjustFrame(HWND hwnd);

LRESULT CALLBACK MifViewWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_SETCURSOR:
			return mv_OnSetCursor(hwnd, (HWND)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));

		case WM_LBUTTONDOWN:
			return mv_OnLButtonDown(hwnd, (UINT)wParam, CPoint(lParam));

		case WM_NCCREATE:
			((LPCREATESTRUCT)lParam)->dwExStyle |= WS_EX_CLIENTEDGE;
			return TRUE;

		case WM_CREATE:
			return mv_OnCreate(hwnd, (LPCREATESTRUCT)lParam);

		case WM_SIZE:
			mv_OnSize(hwnd, (UINT)wParam, (int)LOWORD(lParam), (int)HIWORD(lParam));
			break;

		case WM_PAINT:
			mv_OnPaint(hwnd);
			return 0;

		case WM_PALETTECHANGED:
			mv_OnPaletteChanged(hwnd, (HWND)wParam);
			return 0;

		case WM_QUERYNEWPALETTE:
			return mv_OnQueryNewPalette(hwnd);

		case WM_ERASEBKGND:
			return mv_OnEraseBkgnd(hwnd, (HDC)wParam);

		case WM_DESTROY:
			mv_OnDestroy(hwnd);
			break;

		case IVM_SETDIB:
			return mv_OnSetDIB(hwnd, (HDIB)lParam, (BOOL)wParam);

		case IVM_SETRANGE:
			return mv_OnSetRange(hwnd, (LPRECT)lParam, (BOOL)wParam);

		case IVM_SETPOS:
			return mv_OnSetPos(hwnd, CPoint(lParam));

		case IVM_SETBKCOLOR:
			return mv_OnSetBkColor(hwnd, (COLORREF)lParam, (BOOL)wParam);

		case IVM_UPDATEEFFECT:
			return mv_OnUpdateEffect(hwnd, (BOOL)wParam);

		default:
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static BOOL mv_OnSetCursor(HWND hwnd, HWND hwndCursor, UINT nHitTest, UINT message)
{
	MVINFO* pmi = GETMVINFO(hwnd);
	if (pmi->bCaptured)
	{
		SetCursor(pmi->hcur[1]);
		return TRUE;
	}
	else if (GetKeyState(VK_CONTROL) < 0)
	{
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		return TRUE;
	}
	return DefWindowProc(hwnd, WM_SETCURSOR,
						(WPARAM)hwndCursor, MAKELPARAM(nHitTest, message));
}

static LRESULT mv_OnLButtonDown(HWND hwnd, UINT nFlags, CPoint pt)
{
	MVINFO* pmi = GETMVINFO(hwnd);
	if (GetWindowStyle(hwnd) & IVS_EFFECTCALLBACK)
	{
		UINT id = GetWindowID(hwnd);

		MV_LBUTTONDOWN mvl;
		memset(&mvl, 0, sizeof(mvl));
		mvl.hdr.hwndFrom = hwnd;
		mvl.hdr.idFrom = id;
		mvl.hdr.code = MVN_LBUTTONDOWN;
		mvl.fwKeys = nFlags;
		mvl.ptClient = pt;
		mvl.ptPicture.x = pmi->rcFrame.left + pt.x;
		mvl.ptPicture.y += pmi->rcFrame.top + pt.y;
		if (!SendMessage(GetParent(hwnd), WM_NOTIFY, id, (LPARAM)&mvl))
		{
			mv_TrackMouse(hwnd, pt);
			return 0;
		}
	}
	return DefWindowProc(hwnd, WM_LBUTTONDOWN,
						(WPARAM)nFlags, MAKELPARAM(pt.x, pt.y));
}

static int mv_OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
	MVINFO* pmi = new MVINFO;
	memset(pmi, 0, sizeof(MVINFO));
	pmi->rgbBkgnd = RGB(64,128,128);
	pmi->hcur[0] = LoadCursor(ghInst, MAKEINTRESOURCE(IDC_HANDOPEN));
	pmi->hcur[1] = LoadCursor(ghInst, MAKEINTRESOURCE(IDC_HANDCLOSE));
	SETMVINFO(hwnd, pmi);

	if (lpcs->style & IVS_EFFECTCALLBACK)
		SetClassLong(hwnd, GCL_HCURSOR, (long)pmi->hcur[0]);

	return 0;
}

static void mv_OnSize(HWND hwnd, UINT type, int cx, int cy)
{
	MVINFO* pmi = GETMVINFO(hwnd);
	if (pmi->hbmBack)
		DeleteObject(pmi->hbmBack);

	mif image(cx, cy, 24);
	RGBA* p = image.m_pixels;
	for (int y = 0; y < cy; y++)
		for (int x = 0; x < cx; x++)
		{
			int c = (x/8+y/8)%2==0?0xcc:0xff;
			p->red=p->green=p->blue=c;
			p++;
		}
	pmi->hbmBack = image.createbitmap(NULL);
}

static void mv_OnPaint(HWND hwnd)
{
	MVINFO* pmi = GETMVINFO(hwnd);

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	BITMAP bm;
	CRect rc;
	GetClientRect(hwnd, &rc);

	pmi->bUseEffect = FALSE;
	if (GetWindowStyle(hwnd) & IVS_EFFECTCALLBACK)
	{
		if (pmi->hbmEffect)
		{
			pmi->bUseEffect = TRUE;
			GetObject(pmi->hbmEffect, sizeof(BITMAP), &bm);
			imgDrawBitmap(hdc,
					(rc.left + rc.right - bm.bmWidth) / 2,
					(rc.top + rc.bottom - bm.bmHeight) / 2,
					pmi->hbmEffect, pmi->hpalEffect);
		}
		else
			imgPaintBitmap(hdc, &rc, pmi->hbm, &pmi->rc, pmi->hpal);
	}
	else if (pmi->hbm)
	{
		GetObject(pmi->hbm, sizeof(BITMAP), &bm);
		imgDrawBitmap(hdc,
					(rc.left + rc.right - bm.bmWidth) / 2,
					(rc.top + rc.bottom - bm.bmHeight) / 2,
					pmi->hbm, pmi->hpal);
	}
	EndPaint(hwnd, &ps);
}

static void mv_OnPaletteChanged(HWND hwnd, HWND hwndFocus)
{
	MVINFO* pmi = GETMVINFO(hwnd);

	HPALETTE hpal = pmi->bUseEffect ? pmi->hpalEffect : pmi->hpal;
	if (hpal)
	{
		if (hwnd != hwndFocus)
		{
			HDC hdc = GetDC(hwnd);
			HPALETTE hpalw = SelectPalette(hdc, hpal, FALSE);
			RealizePalette(hdc);
			UpdateColors(hdc);
			SelectPalette(hdc, hpalw, FALSE);
			ReleaseDC(hwnd, hdc);
		}
	}
}

static BOOL mv_OnQueryNewPalette(HWND hwnd)
{
	MVINFO* pmi = GETMVINFO(hwnd);

	HPALETTE hpal = pmi->bUseEffect ? pmi->hpalEffect : pmi->hpal;
	if (hpal)
	{
		HDC hdc = GetDC(hwnd);
		HPALETTE hpalw = SelectPalette(hdc, hpal, FALSE);
		RealizePalette(hdc);

		InvalidateRect(hwnd, NULL, TRUE);
		UpdateWindow(hwnd);

		SelectPalette(hdc, hpalw, FALSE);
		ReleaseDC(hwnd, hdc);
		return TRUE;
	}
	return FALSE;
}

static BOOL mv_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
	MVINFO* pmi = GETMVINFO(hwnd);

	CRect rc;
	GetClientRect(hwnd, &rc);

	HDC hdcw = CreateCompatibleDC(hdc);
	HBITMAP hbm = SelectBitmap(hdcw, pmi->hbmBack);
	BitBlt(hdc, 0, 0, rc.Width(), rc.Height(), hdcw, 0, 0, SRCCOPY);
	SelectBitmap(hdcw, hbm);
	DeleteDC(hdcw);
	return TRUE;
}

static void mv_OnDestroy(HWND hwnd)
{
	MVINFO* pmi = GETMVINFO(hwnd);

	if (pmi->hbm)
		DeleteObject(pmi->hbm);
	if (pmi->hpal)
		DeleteObject(pmi->hpal);
	if (pmi->hbmEffect)
		DeleteObject(pmi->hbmEffect);
	if (pmi->hpalEffect)
		DeleteObject(pmi->hpalEffect);
	if (pmi->hbmBack)
		DeleteObject(pmi->hbmBack);

	delete pmi;
}

static LRESULT mv_OnSetRange(HWND hwnd, LPRECT prc, BOOL bRedraw)
{
	MVINFO* pmi = GETMVINFO(hwnd);

	CPoint pt(0, 0);
	if (prc)
	{
		CopyRect(&pmi->rcFrame, prc);
//		mv_AdjustFrame(hwnd);

		CRect rc, rcFrame = *prc;
		GetClientRect(hwnd, &rc);
		pt.x = rcFrame.left + (rcFrame.Width() - rc.Width()) / 2;
		pt.y = rcFrame.top + (rcFrame.Height() - rc.Height()) / 3;
	}
	else
		SetRectEmpty(&pmi->rcFrame);

	mv_OnSetPos(hwnd, pt);
	return 0;
}

static LRESULT mv_OnSetPos(HWND hwnd, CPoint pt)
{
	MVINFO* pmi = GETMVINFO(hwnd);
	pmi->pt = pt;

	CRect rc;
	GetClientRect(hwnd, &rc);
	pmi->rc.left = pt.x;
	pmi->rc.top = pt.y;
	pmi->rc.right = pt.x + rc.right;
	pmi->rc.bottom = pt.y + rc.bottom;

	return 0;
}

static LRESULT mv_OnSetDIB(HWND hwnd, HDIB hdib, BOOL bRedraw)
{
	MVINFO* pmi = GETMVINFO(hwnd);

	if (pmi->hbm)
		DeleteObject(pmi->hbm);
	if (pmi->hpal)
		DeleteObject(pmi->hpal);
	if (pmi->hbmEffect)
		DeleteObject(pmi->hbmEffect);
	if (pmi->hpalEffect)
		DeleteObject(pmi->hpalEffect);
	pmi->hdib = NULL;
	pmi->width = 0;
	pmi->height = 0;
	pmi->colors = 0;
	pmi->bits = 0;
	pmi->hbm = NULL;
	pmi->hpal = NULL;
	pmi->hbmEffect = NULL;
	pmi->hpalEffect = NULL;

	if (hdib)
	{
		pmi->hdib = hdib;
		dib_getinfo(hdib, &pmi->width, &pmi->height, &pmi->colors, &pmi->bits);

		CRect rc;
		GetClientRect(hwnd, &rc);
		CopyRect(&pmi->rc, &rc);
		if (!(GetWindowStyle(hwnd) & IVS_EFFECTCALLBACK))
		{
			HDIB h = imgScaleInFrame(pmi->hdib, rc.Width(), rc.Height());
			if (h)
			{
				imgCreateBitmapFromDIB(h, &pmi->hbm, &pmi->hpal);
				GlobalFree(h);
			}
		}
		else
			imgCreateBitmapFromDIB(hdib, &pmi->hbm, &pmi->hpal);
	}

	InvalidateRect(hwnd, NULL, bRedraw);
	UpdateWindow(hwnd);
	return 0;
}

static LRESULT mv_OnSetBkColor(HWND hwnd, COLORREF rgb, BOOL bRedraw)
{
	MVINFO* pmi = GETMVINFO(hwnd);

	COLORREF rgbOld = pmi->rgbBkgnd;
	pmi->rgbBkgnd = rgb;
	if (bRedraw)
		InvalidateRect(hwnd, NULL, TRUE);
	return (LRESULT)rgbOld;
}

static LRESULT mv_OnUpdateEffect(HWND hwnd, BOOL bRedraw)
{
	mv_DoUpdateEffect(hwnd);
	if (bRedraw)
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	return 0;
}

static void mv_TrackMouse(HWND hwnd, CPoint pt)
{
	MVINFO* pmi = GETMVINFO(hwnd);

	CRect rcRange, rcc, rcFrame, rc, rcw;
	GetClientRect(hwnd, &rcc);
	rcFrame = pmi->rcFrame;

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
	rcFrame.OffsetRect(-pmi->pt.x, -pmi->pt.y);

	SetCapture(hwnd);
	SetCursor(pmi->hcur[1]);
	pmi->bCaptured = TRUE;

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

			if (rcw != pmi->rc)
			{
				pmi->rc = rcw;
				imgPaintBitmap(hdc, &rcc, pmi->hbm, &pmi->rc, pmi->hpal);

				MV_CHANGEVIEW mvc;
				memset(&mvc, 0, sizeof(mvc));
				mvc.hdr.hwndFrom = hwnd;
				mvc.hdr.idFrom = GetWindowID(hwnd);
				mvc.hdr.code = MVN_CHANGEVIEW;
				mvc.ptTopLeft.x = pmi->rc.left;
				mvc.ptTopLeft.y = pmi->rc.top;
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

	mv_OnSetPos(hwnd, CPoint(pmi->rc.left, pmi->rc.top));
	pmi->bCaptured = FALSE;

	mv_DoUpdateEffect(hwnd);
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

static void mv_DoUpdateEffect(HWND hwnd)
{
	MVINFO* pmi = GETMVINFO(hwnd);

	if (!pmi->hdib)
		return;

	// 新しい画像を問い合わせる
	UINT id = GetWindowID(hwnd);
	MV_GETEFFECT mve;
	memset(&mve, 0, sizeof(mve));
	mve.hdr.hwndFrom = hwnd;
	mve.hdr.idFrom = id;
	mve.hdr.code = MVN_GETEFFECT;

	mif image(pmi->hdib);
	image.crop(&pmi->rc);
	mve.hdibSrc = image.createdib();
	image.destroy();

	SendMessage(GetParent(hwnd), WM_NOTIFY, id, (LPARAM)&mve);
	if (mve.hdibDst)
	{
		if (pmi->hbmEffect)
		{
			DeleteBitmap(pmi->hbmEffect);
			pmi->hbmEffect = NULL;
		}
		if (pmi->hpalEffect)
		{
			DeletePalette(pmi->hpalEffect);
			pmi->hpalEffect = NULL;
		}

		imgCreateBitmapFromDIB(mve.hdibDst, &pmi->hbmEffect, &pmi->hpalEffect);
		GlobalFree(mve.hdibSrc);
		GlobalFree(mve.hdibDst);
	}
}

static void mv_AdjustFrame(HWND hwnd)
{
	MVINFO* pmi = GETMVINFO(hwnd);

	CRect rcc, rcw;
	GetClientRect(hwnd, &rcc);
	CRect rc = pmi->rcFrame;
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


#endif
