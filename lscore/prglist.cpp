// prglist.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"

#ifdef USE_PRGLIST


#define IDC_LIST					100			// inner tree control id

typedef struct tagPRGITEM {
	LPTSTR pText;
	UINT nState;
} PRGITEM;


/////////////////////////////////////////////////////////////////////////////
//

typedef struct tagPRGINFO {
	HIMAGELIST him;
	HBRUSH hbr;
} PRGINFO;

#define SETINFO(a,b)			(SetWindowLongPtr((a), 0, (LONG_PTR)(b)))
#define GETINFO(a)				((PRGINFO*)GetWindowLongPtr((a), 0))

static int pl_OnCreate(HWND hwnd, LPCREATESTRUCT lpcs);
static void pl_OnSize(HWND hwnd, UINT nType, int cx, int cy);
static void pl_OnMeasureItem(HWND hwnd, UINT idCtrl, LPMEASUREITEMSTRUCT lpmi);
static void pl_OnDrawItem(HWND hwnd, UINT idCtrl, LPDRAWITEMSTRUCT lpdi);
static void pl_OnDeleteItem(HWND hwnd, UINT idCtrl, LPDELETEITEMSTRUCT lpds);
static void pl_OnDestroy(HWND hwnd);
static void pl_OnAdd(HWND hwnd, UINT nState, LPCTSTR lpText);
static void pl_OnDelete(HWND hwnd, int nIndex);
static void pl_OnSetState(HWND hwnd, int nIndex, UINT nState);

extern "C"
LRESULT CALLBACK PrgListWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
			return (LRESULT)pl_OnCreate(hwnd, (LPCREATESTRUCT)lParam);

		case WM_SIZE:
			pl_OnSize(hwnd, (UINT)wParam, (int)LOWORD(lParam), (int)HIWORD(lParam));
			break;

		case WM_COMMAND:
			break;

		case WM_MEASUREITEM:
			pl_OnMeasureItem(hwnd, (UINT)wParam, (LPMEASUREITEMSTRUCT)lParam);
			return 0;

		case WM_DRAWITEM:
			pl_OnDrawItem(hwnd, (UINT)wParam, (LPDRAWITEMSTRUCT)lParam);
			return 0;

		case WM_DELETEITEM:
			pl_OnDeleteItem(hwnd, (UINT)wParam, (LPDELETEITEMSTRUCT)lParam);
			return 0;

		case WM_CTLCOLORLISTBOX:
		{
			PRGINFO* prg = GETINFO(hwnd);
			return (LRESULT)prg->hbr;
		}

		case WM_DESTROY:
			pl_OnDestroy(hwnd);
			break;

		case PLM_ADD:
			pl_OnAdd(hwnd, (UINT)wParam, (LPCTSTR)lParam);
			return 0;

		case PLM_DELETE:
			pl_OnDelete(hwnd, (int)wParam);
			return 0;

		case PLM_SETSTATE:
			pl_OnSetState(hwnd, (int)wParam, (UINT)lParam);
			return 0;

		default:
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static int pl_OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
	DWORD style = WS_CHILD | WS_VISIBLE | LBS_OWNERDRAWFIXED | LBS_EXTENDEDSEL | LBS_NOINTEGRALHEIGHT;
	HWND hctl = CreateWindow(WC_LISTBOX, _T(""), style, 0, 0, lpcs->cx, lpcs->cy, hwnd, (HMENU)IDC_LIST, MODINST, NULL);
	if (!hctl)
		return -1;

	HFONT hf = GetWindowFont(GetParent(hwnd));
	SetWindowFont(hctl, hf, FALSE);
	EnableWindow(hctl, FALSE);

	PRGINFO* prg = new PRGINFO;
	memset(prg, 0, sizeof(PRGINFO));
	prg->him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_PRGSTAT), 21, RGB(255, 0, 255));
	prg->hbr = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	SETINFO(hwnd, prg);

	return 0;
}

static void pl_OnSize(HWND hwnd, UINT nType, int cx, int cy)
{
	MoveWindow(GetDlgItem(hwnd, IDC_LIST), 0, 0, cx, cy, TRUE);
}

static void pl_OnMeasureItem(HWND hwnd, UINT idCtrl, LPMEASUREITEMSTRUCT lpmi)
{
	HWND hctl = GetDlgItem(hwnd, idCtrl);
	CClientDC dc(hctl);
	TEXTMETRIC tm;
	GetTextMetrics(dc, &tm);
	lpmi->itemHeight = max(tm.tmHeight + tm.tmExternalLeading, 16) + 2;
}

static void pl_OnDrawItem(HWND hwnd, UINT idCtrl, LPDRAWITEMSTRUCT lpdi)
{
	PRGITEM* ppi = (PRGITEM*)lpdi->itemData;
	if (!ppi || !ppi->pText)
		return;

	PRGINFO* prg = GETINFO(hwnd);
	CRect rc = lpdi->rcItem;
	if (lpdi->itemAction & (ODA_DRAWENTIRE | ODA_SELECT))
	{
		lccPatB(lpdi->hDC, rc.left, rc.top, rc.Width(), rc.Height(),
			 GetSysColor(COLOR_3DFACE));
		ImageList_Draw(prg->him, ppi->nState, lpdi->hDC,
					rc.left + 2, rc.top + (rc.Height() - 16) / 2, ILD_TRANSPARENT);
		rc.left += 20;

		SetTextColor(lpdi->hDC, GetSysColor(COLOR_WINDOWTEXT));
		SetBkColor(lpdi->hDC, GetSysColor(COLOR_3DFACE));
		SetBkMode(lpdi->hDC, TRANSPARENT);
		DrawText(lpdi->hDC, ppi->pText, -1, &rc,
				DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
		if (ppi->nState == PLITEM_CONT)
		{
			rc.left++;
			DrawText(lpdi->hDC, ppi->pText, -1, &rc,
					DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
		}
	}
}

static void pl_OnDeleteItem(HWND hwnd, UINT idCtrl, LPDELETEITEMSTRUCT lpds)
{
	PRGITEM* ppi = (PRGITEM*)lpds->itemData;
	if (ppi) {
		if (ppi->pText) LocalFree(ppi->pText);
		free(ppi);
	}
}

static void pl_OnDestroy(HWND hwnd)
{
	PRGINFO* prg = GETINFO(hwnd);
	if (prg) {
		if (prg->him) ImageList_Destroy(prg->him);
		if (prg->hbr) DeleteObject(prg->hbr);
		delete prg;
	}
}

static void pl_OnAdd(HWND hwnd, UINT nState, LPCTSTR pText)
{
	PRGITEM* ppi = (PRGITEM*)malloc(sizeof(PRGITEM));
	if (pText && *pText)
		ppi->pText = StrDup(pText);
	else
		ppi->pText = NULL;
	ppi->nState = nState;

	CListBox list = GetDlgItem(hwnd, IDC_LIST);
	list.AddString((LPTSTR)ppi);
}

static void pl_OnDelete(HWND hwnd, int nIndex)
{
	CListBox list = GetDlgItem(hwnd, IDC_LIST);
	int cnt = list.GetCount();
	if (0 <= nIndex && nIndex < cnt)
		list.DeleteString(nIndex);
}

static void pl_OnSetState(HWND hwnd, int nIndex, UINT nState)
{
	CListBox list = GetDlgItem(hwnd, IDC_LIST);
	int cnt = list.GetCount();
	if (0 <= nIndex && nIndex < cnt) {
		PRGITEM* ppi = (PRGITEM*)list.GetItemData(nIndex);
		if (ppi) {
			ppi->nState = nState;
			RECT rc;
			list.GetItemRect(nIndex, &rc);
			list.RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			list.SetCaretIndex(nIndex);
		}
	}
}


#endif			// USE_PRGLIST
