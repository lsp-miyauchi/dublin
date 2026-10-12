// graph.cpp : implementation file
//

#include "stdafx.h"
#include "lsimg.h"
#include "resource.h"


extern HINSTANCE ghInst;

///////////////////////////////////////////////////////
//

typedef struct tagGHINFO {
	int table[256];
	int ref;
	int val;
} GHINFO;

///////////////////////////////////////////////////////
// Preview control implementation

#define GETGHINFO(n)			((GHINFO*)GetWindowLong((n), 0))
#define SETGHINFO(n,x)			(SetWindowLong((n), 0, (long)(x)))

static BOOL gh_OnSetCursor(HWND hwnd, HWND hwndCursor, UINT nHitTest, UINT message);
static LRESULT gh_OnLButtonDown(HWND hwnd, UINT nFlags, CPoint pt);
static int gh_OnCreate(HWND hwnd, LPCREATESTRUCT lpcs);
static void gh_OnPaint(HWND hwnd);
static void gh_DoPaint(HWND hwnd, HDC hdc);
static void gh_OnDestroy(HWND hwnd);
static BOOL gh_OnEraseBkgnd(HWND hwnd, HDC hdc);
static void gh_TrackMouse(HWND hwnd, CPoint pt);
static void gh_DoUpdate(HWND hwnd, int ref, int val);
static void gh_AdjustFrame(HWND hwnd);
static LRESULT gh_OnSetTone(HWND hwnd, int ref, int val);

LRESULT CALLBACK GraphWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_SETCURSOR:
			return gh_OnSetCursor(hwnd, (HWND)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));

		case WM_LBUTTONDOWN:
			return gh_OnLButtonDown(hwnd, (UINT)wParam, CPoint(lParam));

		case WM_NCCREATE:
			((LPCREATESTRUCT)lParam)->dwExStyle |= WS_EX_CLIENTEDGE;
			return TRUE;

		case WM_CREATE:
			return gh_OnCreate(hwnd, (LPCREATESTRUCT)lParam);

		case WM_PAINT:
			gh_OnPaint(hwnd);
			return 0;

		case WM_DESTROY:
			gh_OnDestroy(hwnd);
			break;

		case WM_ERASEBKGND:
			return gh_OnEraseBkgnd(hwnd, (HDC)wParam);

		case MGM_SETTONE:
			return gh_OnSetTone(hwnd, (int)LOWORD(lParam), (int)HIWORD(lParam));

		default:
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static BOOL gh_OnSetCursor(HWND hwnd, HWND hwndCursor, UINT nHitTest, UINT message)
{
	GHINFO* pgi = GETGHINFO(hwnd);
	return DefWindowProc(hwnd, WM_SETCURSOR,
						(WPARAM)hwndCursor, MAKELPARAM(nHitTest, message));
}

static LRESULT gh_OnLButtonDown(HWND hwnd, UINT nFlags, CPoint pt)
{
	gh_TrackMouse(hwnd, pt);
	return 0;
}

static int gh_OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
	GHINFO* pgi = new GHINFO;
	for (int i = 0; i < 256; i++)
		pgi->table[i] = i;
	pgi->ref = 128;
	pgi->val = 128;
	SETGHINFO(hwnd, pgi);

	gh_AdjustFrame(hwnd);
	return 0;
}

static void gh_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	gh_DoPaint(hwnd, hdc);
	EndPaint(hwnd, &ps);
}

static void gh_DoPaint(HWND hwnd, HDC hdc)
{
	register int i, j;
	GHINFO* pgi = GETGHINFO(hwnd);
	POINT poly[128];

	poly[0].x = 0;
	poly[0].y = 0;
	j = 1;
	for (i = 4; i < 256; i += 4)
	{
		poly[j].x = i;
		poly[j].y = pgi->table[i];
		j++;
	}
	poly[j].x = 255;
	poly[j].y = 255;
	j++;

	int rop = SetROP2(hdc, R2_XORPEN);
	HPEN hpen = CreatePen(PS_SOLID, 0, RGB(255, 255, 255));
	HPEN hold = SelectPen(hdc, hpen);
	Polyline(hdc, poly, j);
	SelectObject(hdc, hold);
	DeletePen(hpen);
	SetROP2(hdc, rop);

	PatBlt(hdc, pgi->ref - 4, pgi->val - 4, 10, 10, DSTINVERT);
}

static BOOL gh_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
	DefWindowProc(hwnd, WM_ERASEBKGND, (WPARAM)hdc, 0);

	CRect rc;
	GetClientRect(hwnd, &rc);

	LOGBRUSH lbr;
	lbr.lbStyle = BS_SOLID;
	lbr.lbColor = RGB(128, 128, 128);
	lbr.lbHatch = 0;
	HPEN hpen = ExtCreatePen(PS_COSMETIC | PS_ALTERNATE, 1, &lbr, 0, NULL);
	HPEN hold = SelectPen(hdc, hpen);

	POINT ptw;
	register int i, n;
	for (i = 1; i < 4; i++)
	{
		n = 64 * i;
		MoveToEx(hdc, 0, n, &ptw);
		LineTo(hdc, 256, n);

		MoveToEx(hdc, n, 0, &ptw);
		LineTo(hdc, n, 256);
	}

	SelectObject(hdc, hold);
	DeletePen(hpen);

	return TRUE;
}

static void gh_OnDestroy(HWND hwnd)
{
	GHINFO* pgi = GETGHINFO(hwnd);
	delete pgi;
}

static void gh_TrackMouse(HWND hwnd, CPoint pt)
{
	GHINFO* pgi = GETGHINFO(hwnd);

	CRect rc;
	GetClientRect(hwnd, &rc);
	int line = min(rc.Width(), rc.Height());
	int ref, val;

	UINT id = GetWindowID(hwnd);
	MG_SELCHANGE mg;
	memset(&mg, 0, sizeof(mg));
	mg.hdr.hwndFrom = hwnd;
	mg.hdr.idFrom = id;
	mg.hdr.code = MGN_SELCHANGE;

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

			GetCursorPos(&ptw);
			ScreenToClient(hwnd, &ptw);
			DPtoLP(hdc, &ptw, 1);

			ref = min(254, max(1, ptw.x));
			val = ptw.y;

			gh_DoPaint(hwnd, hdc);
			pgi->ref = ref;
			pgi->val = val;
			imgLagFill(pgi->table, ref, val);
			gh_DoPaint(hwnd, hdc);

			mg.ref = ref;
			mg.val= val;
			SendMessage(GetParent(hwnd), WM_NOTIFY, id, (LPARAM)&mg);

			if (msg.message == WM_LBUTTONUP)
				break;
		}
		else
			continue;
	}
	ReleaseDC(hwnd, hdc);

	gh_DoUpdate(hwnd, ref, val);
}

static void gh_DoUpdate(HWND hwnd, int ref, int val)
{
	GHINFO* pgi = GETGHINFO(hwnd);

	imgLagFill(pgi->table, ref, val);
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);

	UINT id = GetWindowID(hwnd);
	MG_SELCHANGE mg;
	memset(&mg, 0, sizeof(mg));
	mg.hdr.hwndFrom = hwnd;
	mg.hdr.idFrom = id;
	mg.hdr.code = MGN_SELCHANGE;
	mg.ref = ref;
	mg.val= val;
	SendMessage(GetParent(hwnd), WM_NOTIFY, id, (LPARAM)&mg);
}

static void gh_AdjustFrame(HWND hwnd)
{
	CRect rcc, rcw;
	GetClientRect(hwnd, &rcc);
	GetWindowRect(hwnd, &rcw);

	int sqc = min(rcc.Width(), rcc.Height());
	int sqw = min(rcw.Width(), rcw.Height());
	int delta = sqw - sqc;
	sqc = (sqc >> 2) << 2;
	sqw = sqc + delta;
	rcw.right = rcw.left + sqw;
	rcw.bottom = rcw.top + sqw;

	HWND hParent = GetParent(hwnd);
	ScreenToClient(hParent, (LPPOINT)&rcw.left);
	ScreenToClient(hParent, (LPPOINT)&rcw.right);
	MoveWindow(hwnd, rcw.left, rcw.top, rcw.Width(), rcw.Height(), TRUE);

	SIZE sz;
	HDC hdc = GetDC(hwnd);
	SetMapMode(hdc, MM_ISOTROPIC);
	SetWindowExtEx(hdc, 255, 255, &sz);
	SetViewportExtEx(hdc, sqc, -sqc, &sz);
	SetViewportOrgEx(hdc, 0, sqc - 1, (LPPOINT)&sz);
}

static LRESULT gh_OnSetTone(HWND hwnd, int ref, int val)
{
	GHINFO* pgi = GETGHINFO(hwnd);

	ref = min(254, max(1, ref));
	pgi->ref = ref;
	pgi->val = val;
	imgLagFill(pgi->table, ref, val);
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	return 0;
}
